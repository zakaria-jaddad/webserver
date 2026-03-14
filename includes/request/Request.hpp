#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include "../config/Config.hpp"
#include "../response/ResolvedConfig.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <ios>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define CRLF "\r\n"
#define DOUBLE_CRLF "\r\n\r\n"
#define GREEN "\033[32m"
#define WHITE "\033[0m"

using namespace std;

/* inline only for development */
inline std::string trim(const std::string &str)
{
    size_t start = 0;
    while (start < str.length() && (str[start] == ' ' || str[start] == '\t')) {
        ++start;
    }

    if (start == str.length()) {
        return "";
    }

    size_t end = str.length() - 1;
    while (end > start && (str[end] == ' ' || str[end] == '\t')) {
        --end;
    }

    return str.substr(start, end - start + 1);
}

inline bool is_all_digits(const std::string str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
}

inline int hex_to_decimal(string hex)
{
    int decimal = 0;
    int base = 1;
    for (int i = hex.length() - 1; i >= 0; i--) {
        if (hex[i] >= '0' && hex[i] <= '9') {
            decimal += (hex[i] - '0') * base;
        }
        else if (hex[i] >= 'A' && hex[i] <= 'F') {
            decimal += (hex[i] - 'A' + 10) * base;
        }
        else if (hex[i] >= 'a' && hex[i] <= 'f') {
            decimal += (hex[i] - 'a' + 10) * base;
        }
        else {
            return -1;
        }
        base *= 16;
    }
    return decimal;
}

/*
 *
 * The syntax of an "HTTP message" is defined:
 *
 * HTTP-message = start-line CRLF
 *                *( field-line CRLF )
 *                CRLF
 *                [ message-body ]
 */
class HttpParser {

  public:
    HttpParser(void) : _phase(REQUEST_LINE), _body()
    {
        this->_requestURI.path = "";
        this->_requestURI.params.clear();
        this->_requestURI.rawURI = "";
        this->_requestURI.rawParams = "";

        this->_requestVersion[0] = 0;
        this->_requestVersion[1] = 0;
        this->_requestType = UNKNOWN;
        this->_config = NULL;
        this->_res = HTTP_RES_OK;
    }

    HttpParser(ServerConfig *ServerConfig) : _phase(REQUEST_LINE), _body()
    {
        this->_requestURI.path = "";
        this->_requestURI.params.clear();
        this->_requestURI.rawURI = "";
        this->_requestURI.rawParams = "";

        this->_requestVersion[0] = 0;
        this->_requestVersion[1] = 0;
        this->_requestType = UNKNOWN;
        this->_config = ServerConfig;
        this->_res = HTTP_RES_OK;
    }

    ~HttpParser() {}

    /* Server Errors */
    typedef enum {
        // ─────────────────────────────────────────────
        // 1xx Informational
        // ─────────────────────────────────────────────
        HTTP_RES_CONTINUE = 100,
        HTTP_RES_SWITCHING_PROTOCOLS = 101,

        // ─────────────────────────────────────────────
        // 2xx Success
        // ─────────────────────────────────────────────
        HTTP_RES_OK = 200,
        HTTP_RES_CREATED = 201,
        HTTP_RES_ACCEPTED = 202,
        HTTP_RES_NO_CONTENT = 204,
        HTTP_RES_PARTIAL_CONTENT = 206,

        // ─────────────────────────────────────────────
        // 3xx Redirection
        // ─────────────────────────────────────────────
        HTTP_RES_MOVED_PERMANENTLY = 301,
        HTTP_RES_FOUND = 302,
        HTTP_RES_SEE_OTHER = 303,
        HTTP_RES_NOT_MODIFIED = 304,
        HTTP_RES_TEMPORARY_REDIRECT = 307,
        HTTP_RES_PERMANENT_REDIRECT = 308,

        // ─────────────────────────────────────────────
        // 4xx Client Errors
        // ─────────────────────────────────────────────
        HTTP_RES_BAD_REQUEST = 400,
        HTTP_RES_UNAUTHORIZED = 401,
        HTTP_RES_FORBIDDEN = 403,
        HTTP_RES_NOT_FOUND = 404,
        HTTP_RES_METHOD_NOT_ALLOWED = 405,
        HTTP_RES_NOT_ACCEPTABLE = 406,
        HTTP_RES_REQUEST_TIMEOUT = 408,
        HTTP_RES_CONFLICT = 409,
        HTTP_RES_GONE = 410,
        HTTP_RES_LENGTH_REQUIRED = 411,
        HTTP_RES_PRECONDITION_FAILED = 412,
        HTTP_RES_REQUEST_ENTITY_TOO_LARGE = 413,
        HTTP_RES_REQUEST_URI_TOO_LONG = 414,
        HTTP_RES_UNSUPPORTED_MEDIA_TYPE = 415,
        HTTP_RES_RANGE_NOT_SATISFIABLE = 416,
        HTTP_RES_EXPECTATION_FAILED = 417,
        HTTP_RES_TOO_MANY_REQUESTS = 429,
        HTTP_RES_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,

        // ─────────────────────────────────────────────
        // 5xx Server Errors
        // ─────────────────────────────────────────────
        HTTP_RES_INTERNAL_SERVER_ERROR = 500,
        HTTP_RES_NOT_IMPLEMENTED = 501,
        HTTP_RES_BAD_GATEWAY = 502,
        HTTP_RES_SERVICE_UNAVAILABLE = 503,
        HTTP_RES_GATEWAY_TIMEOUT = 504,
        HTTP_RES_HTTP_VERSION_NOT_SUPPORTED = 505

    } httpResult;

    static const char *httpResultToString(httpResult result)
    {
        switch (result) {
        // 1xx
        case HTTP_RES_CONTINUE:
            return "100 Continue";
        case HTTP_RES_SWITCHING_PROTOCOLS:
            return "101 Switching Protocols";

        // 2xx
        case HTTP_RES_OK:
            return "200 OK";
        case HTTP_RES_CREATED:
            return "201 Created";
        case HTTP_RES_ACCEPTED:
            return "202 Accepted";
        case HTTP_RES_NO_CONTENT:
            return "204 No Content";
        case HTTP_RES_PARTIAL_CONTENT:
            return "206 Partial Content";

        // 3xx
        case HTTP_RES_MOVED_PERMANENTLY:
            return "301 Moved Permanently";
        case HTTP_RES_FOUND:
            return "302 Found";
        case HTTP_RES_SEE_OTHER:
            return "303 See Other";
        case HTTP_RES_NOT_MODIFIED:
            return "304 Not Modified";
        case HTTP_RES_TEMPORARY_REDIRECT:
            return "307 Temporary Redirect";
        case HTTP_RES_PERMANENT_REDIRECT:
            return "308 Permanent Redirect";

        // 4xx
        case HTTP_RES_BAD_REQUEST:
            return "400 Bad Request";
        case HTTP_RES_UNAUTHORIZED:
            return "401 Unauthorized";
        case HTTP_RES_FORBIDDEN:
            return "403 Forbidden";
        case HTTP_RES_NOT_FOUND:
            return "404 Not Found";
        case HTTP_RES_METHOD_NOT_ALLOWED:
            return "405 Method Not Allowed";
        case HTTP_RES_NOT_ACCEPTABLE:
            return "406 Not Acceptable";
        case HTTP_RES_REQUEST_TIMEOUT:
            return "408 Request Timeout";
        case HTTP_RES_CONFLICT:
            return "409 Conflict";
        case HTTP_RES_GONE:
            return "410 Gone";
        case HTTP_RES_LENGTH_REQUIRED:
            return "411 Length Required";
        case HTTP_RES_PRECONDITION_FAILED:
            return "412 Precondition Failed";
        case HTTP_RES_REQUEST_ENTITY_TOO_LARGE:
            return "413 Payload Too Large";
        case HTTP_RES_REQUEST_URI_TOO_LONG:
            return "414 URI Too Long";
        case HTTP_RES_UNSUPPORTED_MEDIA_TYPE:
            return "415 Unsupported Media Type";
        case HTTP_RES_RANGE_NOT_SATISFIABLE:
            return "416 Range Not Satisfiable";
        case HTTP_RES_EXPECTATION_FAILED:
            return "417 Expectation Failed";
        case HTTP_RES_TOO_MANY_REQUESTS:
            return "429 Too Many Requests";
        case HTTP_RES_REQUEST_HEADER_FIELDS_TOO_LARGE:
            return "431 Request Header Fields Too Large";

        // 5xx
        case HTTP_RES_INTERNAL_SERVER_ERROR:
            return "500 Internal Server Error";
        case HTTP_RES_NOT_IMPLEMENTED:
            return "501 Not Implemented";
        case HTTP_RES_BAD_GATEWAY:
            return "502 Bad Gateway";
        case HTTP_RES_SERVICE_UNAVAILABLE:
            return "503 Service Unavailable";
        case HTTP_RES_GATEWAY_TIMEOUT:
            return "504 Gateway Timeout";
        case HTTP_RES_HTTP_VERSION_NOT_SUPPORTED:
            return "505 HTTP Version Not Supported";

        default:
            return "Unknown Status";
        }
    }

    typedef enum {
        GET,
        POST,
        DELETE,
        HEAD,
        PUT,
        OPTIONS,
        TRACE,
        CONNECT,
        UNKNOWN,
    } requestType;

    httpResult parseHttpRequest(std::string rawData)
    {

        httpResult res;
        // cout << " ==== " << endl;
        // std::cout << rawData << std::endl;
        // cout << " ==== " << endl;

        if (this->_phase == REQUEST_LINE) {

            std::string requestLine = rawData.substr(0, rawData.find(CRLF));
            rawData.erase(0, requestLine.size() + strlen(CRLF));

            /* parse Request-Line ... */
            res = _parseRequestLine(requestLine);
            if (res != HTTP_RES_OK) {
                return res;
            }

            this->_phase = HEADERS;
        }

        if (this->_phase == HEADERS) {

            std::string headerLines = rawData;

            size_t doubleCrlfPos = rawData.find(DOUBLE_CRLF);

            if (doubleCrlfPos != std::string::npos) {
                // WARNING: A + 2 was after doubleCrlfPos no idea why it was there.
                // removed it to get the first body part
                headerLines = rawData.substr(0, doubleCrlfPos);
                rawData.erase(0, headerLines.size() + strlen(DOUBLE_CRLF));
                this->_phase = BODY;
            }

            res = _generateHeadersMap(headerLines, this->_headers);
            if (res != HTTP_RES_OK) {
                return res;
            }
        }

        if (this->_phase == BODY) {

            if (this->_body._state == Body::BODY_NOT_STARTED) {

                std::cout << "Parsing Headers" << std::endl;
                res = _parseRequestHeaderValue();
                std::cout << "headers Parsed, res: ";
                printHttpResult(res);
                if (res != HTTP_RES_OK) {
                    return res;
                }

                /* If request type other than POST ignore the body */
                if (this->_requestType != POST) {
                    this->_phase = COMPLETE;
                    return HTTP_RES_OK;
                }

                this->_body.init(this);
            }

            std::cout << "Parsing Body" << std::endl;
            res = this->_body.parseRequestBody(rawData);
            std::cout << "body Parsed, res: ";
            printHttpResult(res);
            if (res != HTTP_RES_OK) {
                return res;
            }

            if (this->_body._state == Body::BODY_COMPLETE) {
                this->_phase = COMPLETE;
            }
        }

        return HTTP_RES_OK;
    };

    // Getters
    requestType getRequestType() const { return this->_requestType; }
    const int *getRequestVersion() const { return this->_requestVersion; }
    std::map<std::string, std::vector<std::string> > getHeaders() const { return this->_headers; }
    httpResult getRes() const { return this->_res; }

    //
    std::string getRequestUriPath() const { return this->_requestURI.getPath(); }
    std::string getRequestUriRawPath() const { return this->_requestURI.getRawUri(); }
    std::string getRequestUriRawParams() const { return this->_requestURI.getRawParams(); }
    std::map<std::string, std::string> getRequestUriParams() const
    {
        return this->_requestURI.getParams();
    }
    ServerConfig *getServerConfig() const { return this->_config; }

    // Setters
    void setRes(httpResult res) { this->_res = res; }

    bool isComplete() { return this->_phase == HttpParser::COMPLETE; }
    bool isBodyComplete() { return this->_body._state == Body::BODY_COMPLETE; }

  private:
    static char _hexToChar(std::string hex)
    {
        int ret;
        std::stringstream ss;
        ss << std::hex << hex;
        ss >> ret;
        return static_cast<char>(ret);
    }

    static bool _isHex(char c)
    {
        return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
    }

    static bool _isValidPercentEncoding(std::string str)
    {
        if (str.length() != 2)
            return false;
        return _isHex(str[0]) && _isHex(str[1]);
    }

    static std::string _urlDecode(std::string str)
    {
        std::string dec = "";

        for (size_t i = 0; i < str.length(); i++) {
            if (str[i] == '%') {
                if (i + 2 < str.length() && _isValidPercentEncoding(str.substr(i + 1, 2))) {
                    dec += _hexToChar(str.substr(i + 1, 2));
                    i += 2;
                }
                else {
                    return ""; // Error: Invalid or incomplete encoding
                }
            }
            else {
                dec += str[i];
            }
        }
        return dec;
    }

    static void _parseQueryString(std::string &query, std::map<std::string, std::string> &store)
    {
        std::stringstream ss(query);
        std::string param;

        while (getline(ss, param, '&')) {
            std::string key;
            std::string value;
            std::stringstream paramStream(param);

            // Read key until '='
            if (getline(paramStream, key, '=')) {
                // Read the rest of the stream as value
                getline(paramStream, value);

                // docode key and value
                key = _urlDecode(key);
                value = _urlDecode(value);
                store[key] = value;
            }
        }
    }

    typedef struct RequestURI {
      public:
        std::string path;
        std::map<std::string, std::string> params;
        std::string rawURI;
        std::string rawParams;

        std::string getPath() const { return this->path; }
        std::map<std::string, std::string> getParams() const { return this->params; }
        std::string getRawUri() const { return this->rawURI; }
        std::string getRawParams() const { return this->rawParams; }

        HttpParser::httpResult parseRequestURI(std::string uri)
        {
            this->rawURI = uri;
            // this->params.clear(); // Important: Reset params for new request

            size_t pos = uri.find("?");

            if (pos != std::string::npos) {
                // Split URI into Path and Query Params
                this->path = uri.substr(0, pos);
                this->rawParams = uri.substr(pos + 1);

                // Use the shared utility
                HttpParser::_parseQueryString(this->rawParams, this->params);
            }
            else {
                this->path = uri;
                this->rawParams = "";
            }

            // Decode the path (e.g., "%20" -> " ")
            this->path = HttpParser::_urlDecode(this->path);

            if (this->path.empty()) {
                return HttpParser::HTTP_RES_BAD_REQUEST;
            }
            return HttpParser::HTTP_RES_OK;
        }
    } RequestURI;

    typedef enum {
        REQUEST_LINE,
        HEADERS,
        BODY,
        COMPLETE,
    } parserPhase;

    typedef struct Body {

        typedef enum {
            NONE,
            CONTENT_LENGTH,
            CHUNKED,
        } bodyType;

        typedef enum {
            BODY_NOT_STARTED,
            BODY_READING,
            BODY_PROCESSING,
            BODY_COMPLETE,
        } bodyState;

        typedef enum {
            MP_EXPECTING_BOUNDARY,
            MP_READING_HEADERS,
            MP_READING_BODY,
            MP_COMPLETE,
        } multiPartState;

      private:
        void normalizePath(void)
        {
            std::stringstream ss(httpParser->getRequestUriPath());
            std::string elem;
            std::vector<std::string> elements;
            std::string normPath = "";

            while (getline(ss, elem, '/')) {
                if (elem == "." || elem.empty())
                    continue;
                else if (elem == ".." && elements.size() != 0)
                    elements.resize(elements.size() - 1);
                else if (elem != "..")
                    elements.push_back(elem);
            }
            for (std::vector<std::string>::iterator it = elements.begin(); it != elements.end();
                 it++) {
                normPath += "/";
                normPath += *it;
            }
            if (normPath == "")
                normPath = "/";
            // std::cout << "The normalized path is :'" << normPath << "'" << std::endl;
            this->normalizedPath = normPath;
        }

        std::string resolveFilePath(std::string &upload_store)
        {
            std::string uri = this->normalizedPath;
            std::string relative_path = uri;
            std::string location_prefix = this->conf->getLocPath();

            if (uri.find(location_prefix) == 0)
                relative_path = uri.substr(location_prefix.size());

            if (!relative_path.empty() && relative_path[0] == '/')
                relative_path.erase(0, 1);

            std::string final_path = upload_store;
            if (!final_path.empty() && final_path[final_path.size() - 1] != '/')
                final_path += '/';
            final_path += relative_path;

            return final_path;
        }

        httpResult _initParsingInfo()
        {

            std::string uploadPath = this->conf->getUploadPath();

            if (uploadPath.empty()) {
                return HTTP_RES_METHOD_NOT_ALLOWED;
            }

            if (access(uploadPath.c_str(), R_OK) == -1) {
                return HTTP_RES_INTERNAL_SERVER_ERROR;
            }

            this->filePath = resolveFilePath(uploadPath);

            file.open(this->filePath.c_str(), ios::out | ios::in | ios::trunc);
            if (file.is_open() == false) {
                cerr << "Error: " << this->filePath << " Failed to open file" << endl;
                return HTTP_RES_INTERNAL_SERVER_ERROR;
            }

            if (httpParser->_headers.find("Content-Type") != httpParser->_headers.end()) {
                std::string contentType = httpParser->_headers["Content-Type"][0];
                this->_mediaType = contentType;

                if (contentType.find("multipart/form-data") != std::string::npos) {
                    // It MUST have a boundary
                    if (contentType.find("boundary=") == std::string::npos) {
                        this->_fileCleanUp();        // Close/remove the file you just opened
                        return HTTP_RES_BAD_REQUEST; // Reject immediately
                    }

                    // Store the boundary
                    this->_mediaType = contentType.substr(0, contentType.find(";"));
                    this->_boundary = contentType.substr(contentType.find("boundary=") + 9);
                }
            }

            if (httpParser->_headers.find("Transfer-Encoding") == httpParser->_headers.end() &&
                httpParser->_headers.find("Content-Length") == httpParser->_headers.end()) {
                return HTTP_RES_LENGTH_REQUIRED;
            }

            /* Transfer-Encoding Not found */
            if (httpParser->_headers.find("Transfer-Encoding") == httpParser->_headers.end()) {

                std::vector<std::string> contentLength = httpParser->_headers["Content-Length"];
                if (contentLength.size() != 1) {
                    return HTTP_RES_BAD_REQUEST;
                }

                size_t contentLengthValue = std::atoi(contentLength[0].c_str());

                // ignored for now
                if (contentLengthValue <= 0) {
                    return HTTP_RES_LENGTH_REQUIRED;
                }

                if (contentLengthValue > this->_maxBodySize) {
                    return HTTP_RES_REQUEST_ENTITY_TOO_LARGE;
                }
                this->_type = CONTENT_LENGTH;
                this->_contentLength = contentLengthValue;
                return HTTP_RES_OK;
            }

            std::vector<std::string> transferEncoding = httpParser->_headers["Transfer-Encoding"];
            if (transferEncoding.size() != 1) {
                return HTTP_RES_BAD_REQUEST;
            }
            std::string transferEncodingType = transferEncoding[0];
            if (transferEncodingType == "") {
                return HTTP_RES_BAD_REQUEST;
            }
            if (transferEncodingType != "chunked") {
                return HTTP_RES_NOT_IMPLEMENTED;
            }
            this->_type = CHUNKED;
            return HTTP_RES_OK;
        }

        std::string extractPar(const std::string header, std::string key)
        {
            std::string searchTarget = key + "=\"";

            size_t startPos = header.find(searchTarget);
            if (startPos == std::string::npos) {
                return ""; // Key not found
            }

            startPos += searchTarget.length();

            size_t endPos = header.find("\"", startPos);
            if (endPos == std::string::npos) {
                return "";
            }

            return header.substr(startPos, endPos - startPos);
        }

        void _fileCleanUp()
        {
            file.close();
            remove(this->filePath.c_str());
        }

      private:
        /* Content Type Info */
        std::string _boundary;
        std::string _mediaType;
        multiPartState _mpState;

        // For uploaded files
        struct FilePart {
            std::string name;     // Form field name (e.g., "avatar")
            std::string filename; // Original filename (e.g., "me.png")
            std::string contentType;
            std::string filePath; // Path where the final file is saved on server
            std::fstream file;
        };
        FilePart _currentPart;
        std::map<std::string, std::vector<std::string> > _partLevelHeaders;
        std::string _mpCurrentLine;
        std::vector<FilePart> _files;

      public:
        /* General Body Info */
        bodyType _type;
        bodyState _state;
        size_t _maxBodySize;
        std::string _body;
        std::string normalizedPath;
        fstream file;
        string filePath;
        ResolvedConfig *conf;

        /*
         * Transfer-Encoding: chunked
         * Content-Length: <int>
         * Info
         */
        size_t _contentLength;
        size_t _bytesReceived;
        HttpParser *httpParser;

        // Declared size of the chunk currently being read. Used to resume reading when a chunk
        // spans multiple recv() calls
        long currentChunkSize;
        // Number of bytes already read for the current chunk. Compared against currentChunkSize to
        // verify completeness.
        long currentChunkRead;
        // Set to true when the terminating chunk (size 0) is received, signaling that the entire
        // chunked body has been fully read.
        bool currentChunkComplete;
        bool chunkSizeReceived;
        std::string rawChunkSize;

        void printFilePart(const FilePart &part)
        {
            std::cout << "Form Name    : [" << part.name << "]" << std::endl;
            std::cout << "Filename     : [" << part.filename << "]" << std::endl;
            std::cout << "Content-Type : [" << part.contentType << "]" << std::endl;
            std::cout << "Saved Path   : [" << part.filePath << "]" << std::endl;
        }

        Body()
        {
            this->_type = NONE;
            this->_state = BODY_NOT_STARTED;
            this->_contentLength = 0;
            this->_bytesReceived = 0;
            this->httpParser = NULL;
            this->conf = NULL;
            this->_maxBodySize = 0;
            this->_body = "";
            this->normalizedPath = "";
            this->filePath = "";
            this->currentChunkSize = 0;
            this->currentChunkRead = 0;
            this->currentChunkComplete = true;
            this->chunkSizeReceived = true;
            this->rawChunkSize = "";
            this->_body = "";
            this->_mediaType = "";
        }

        ~Body()
        {
            this->file.close();
            delete this->conf;
        }

        void init(HttpParser *httpParser)
        {
            this->httpParser = httpParser;
            this->_maxBodySize = this->httpParser->_config->getSize();

            this->conf = new ResolvedConfig(httpParser->getRequestUriPath(), httpParser->_config);
            normalizePath();
        }

        httpResult parseRequestBody(std::string rawData)
        {
            httpResult res;
            /* If a message is received with both a Transfer-Encoding and a Content-Length
             * header field, the Transfer-Encoding overrides the Content-Length. */
            if (this->_state == BODY_NOT_STARTED) {
                res = _initParsingInfo();
                httpParser->printHttpResult(res);
                if (res != HTTP_RES_OK) {
                    return res;
                }
                this->_state = BODY_READING;
            }

            std::cout << "max body size: " << this->_maxBodySize << std::endl;
            std::cout << "content-length: " << this->_contentLength << std::endl;
            size_t rawDataSize = rawData.size();
            if (this->_state == BODY_READING) {
                std::cout << "\033[33mProcessing Body\033[0m" << std::endl;

                // Content-Length Body Parsing
                if (this->_type == CONTENT_LENGTH) {
                    std::cout << "\033[33mProcessing Body using Content-Length value\033[0m"
                              << std::endl;
                    if (rawDataSize + this->_bytesReceived > this->_contentLength) {
                        this->_fileCleanUp();
                        return HTTP_RES_REQUEST_ENTITY_TOO_LARGE;
                    }
                    file.write(rawData.c_str(), rawDataSize);
                    this->_bytesReceived = this->_bytesReceived + rawDataSize;
                    if (this->_bytesReceived == this->_contentLength) {
                        this->_state = BODY_PROCESSING;
                        this->file.clear();
                        this->file.seekg(0, ios::beg);
                    }
                }

                // Chunked Body Parsing
                if (this->_type == CHUNKED) {

                    size_t crlfPos;
                    if (this->currentChunkComplete == false) {
                        // complete reading the chunk
                        crlfPos = rawData.find(CRLF);
                        if (crlfPos == std::string::npos) {
                            if (this->_bytesReceived + this->currentChunkRead + rawData.size() >
                                this->_maxBodySize) {
                                return HTTP_RES_REQUEST_ENTITY_TOO_LARGE;
                            }
                            file.write(rawData.c_str(), rawData.size());
                            this->_bytesReceived += rawData.size();
                            this->currentChunkRead += rawData.size();
                            return HTTP_RES_OK;
                        }
                        std::string chunkData = rawData.substr(0, crlfPos);
                        this->currentChunkRead += chunkData.size();

                        if (this->currentChunkRead != this->currentChunkSize) {
                            this->_fileCleanUp();
                            return HTTP_RES_BAD_REQUEST;
                        }

                        file.write(chunkData.c_str(), chunkData.size());
                        this->_bytesReceived += chunkData.size();
                        rawData.erase(0, crlfPos + strlen(CRLF));

                        this->currentChunkComplete = true;
                        this->currentChunkRead = 0;
                        this->currentChunkSize = 0;
                    }

                    while ((crlfPos = rawData.find(CRLF)) != std::string::npos) {

                        string encodedChunkSize = rawData.substr(0, crlfPos);
                        if (this->chunkSizeReceived == false) {
                            encodedChunkSize = this->rawChunkSize + encodedChunkSize;

                            // reset
                            this->rawChunkSize = "";
                            this->chunkSizeReceived = true;
                        }

                        rawData.erase(0, encodedChunkSize.size() + strlen(CRLF));
                        long chunkSize = hex_to_decimal(encodedChunkSize);
                        std::cout << "chunkSize: " << chunkSize << std::endl;

                        if (chunkSize == -1) {
                            return HTTP_RES_BAD_REQUEST;
                        }

                        if (chunkSize == 0) {
                            file.close();
                            std::cout << "Finished Chunked Body" << std::endl;
                            this->_state = BODY_PROCESSING;
                            // return HTTP_RES_OK;
                            // this->file.close();
                            this->file.clear();
                            this->file.seekg(0, ios::beg);
                            break;
                        }
                        this->currentChunkSize = chunkSize;

                        if (this->_bytesReceived + this->currentChunkSize > this->_maxBodySize) {
                            this->_fileCleanUp();
                            return HTTP_RES_REQUEST_ENTITY_TOO_LARGE;
                        }

                        crlfPos = rawData.find(CRLF);

                        // if CRLF  Not Found
                        if (crlfPos == std::string::npos) {
                            this->currentChunkComplete = false;
                            if (this->currentChunkRead + long(rawData.size()) >
                                this->currentChunkSize) {
                                this->_fileCleanUp();
                                return HTTP_RES_REQUEST_ENTITY_TOO_LARGE;
                            }
                            file.write(rawData.c_str(), rawData.size());
                            this->_bytesReceived += rawData.size();
                            this->currentChunkRead += rawData.size();
                            return HTTP_RES_OK;
                        }
                        std::string chunkedData = rawData.substr(0, crlfPos);
                        rawData.erase(0, chunkedData.size() + strlen(CRLF));
                        if (this->currentChunkSize != long(chunkedData.size())) {
                            this->_fileCleanUp();
                            return HTTP_RES_BAD_REQUEST;
                        }
                        file.write(chunkedData.c_str(), chunkedData.size());
                        this->_bytesReceived = this->_bytesReceived + chunkedData.size();
                        this->currentChunkRead = 0;
                        this->currentChunkSize = 0;
                    }

                    if (this->_state == BODY_PROCESSING &&
                        rawData.find(CRLF) == std::string::npos) {
                        return HTTP_RES_OK;
                    }

                    // NOTE: I only got the first part of the size
                    if (rawData.empty() == false && rawData.find(CRLF) == std::string::npos) {
                        this->chunkSizeReceived = false;
                        this->rawChunkSize += rawData;
                    }
                }

                if (this->_state == BODY_PROCESSING) {

                    if (this->_mediaType == "application/x-www-form-urlencoded") {

                        std::map<std::string, std::string> params;

                        std::string urlEncodedForm;

                        this->file.seekg(0, ios::beg);
                        this->file >> urlEncodedForm;

                        std::cout << "urlEncodedForm: " << urlEncodedForm << std::endl;

                        _parseQueryString(urlEncodedForm, params);
                        if (params.size() == 0) {
                            return HTTP_RES_BAD_REQUEST;
                        }

                        std::cout << "Url decode form" << std::endl;
                        for (std::map<std::string, std::string>::const_iterator it = params.begin();
                             it != params.end(); it++) {
                            std::cout << "key: " << it->first << ", value: " << it->second
                                      << std::endl;
                        }

                        _fileCleanUp();
                        std::cout << "\033[33mapplication/x-www-form-urlencoded: Parsed\033[0m"
                                  << std::endl;
                        this->_state = BODY_COMPLETE;
                        return HTTP_RES_OK;
                    }

                    if (this->_mediaType == "multipart/form-data") {

                        std::cout << "\033[33mmultipart/form-data Processing\033[0m" << std::endl;
                        std::cout << "this is the boundary: " << this->_boundary << std::endl;

                        std::string line;
                        int linesProcessed = 0;
                        int serverLoop = 30;

                        while (linesProcessed < serverLoop && this->_mpState != MP_COMPLETE) {

                            if (!std::getline(this->file, line)) {
                                return HTTP_RES_BAD_REQUEST;
                            }
                            linesProcessed++;

                            bool hadCr = false;
                            if (line.empty() == false && line[line.size() - 1] == '\r') {
                                line.erase((line.size() - 1));
                                hadCr = true;
                            }

                            if (this->_mpState == MP_EXPECTING_BOUNDARY) {
                                if ("--" + this->_boundary == line) {
                                    this->_mpState = MP_READING_HEADERS;
                                    this->_currentPart = FilePart();
                                    this->_partLevelHeaders.clear();
                                }
                                else if ("--" + this->_boundary + "--" == line) {
                                    this->_mpState = MP_COMPLETE;
                                    this->_state = BODY_COMPLETE;
                                    return HTTP_RES_OK;
                                }
                                continue;
                            }

                            if (this->_mpState == MP_READING_HEADERS) {
                                if (line.empty()) {

                                    if (this->_partLevelHeaders.find("Content-Disposition") ==
                                            this->_partLevelHeaders.end() ||
                                        this->_partLevelHeaders["Content-Disposition"].size() !=
                                            1) {
                                        return HTTP_RES_BAD_REQUEST;
                                    }

                                    std::string contentDispositionValue =
                                        this->_partLevelHeaders["Content-Disposition"][0];
                                    this->_currentPart.name =
                                        extractPar(contentDispositionValue, "name");
                                    this->_currentPart.filename =
                                        extractPar(contentDispositionValue, "filename");
                                    this->_currentPart.filePath = this->conf->getUploadPath() +
                                                                  "/" + this->_currentPart.filename;

                                    // if contentType found add it
                                    if (this->_partLevelHeaders.find("Content-Type") !=
                                            this->_partLevelHeaders.end() &&
                                        this->_partLevelHeaders["Content-Type"].empty() == false) {
                                        this->_currentPart.contentType =
                                            this->_partLevelHeaders["Content-Type"][0];
                                    }

                                    /* open output file */
                                    this->_currentPart.file.open(
                                        this->_currentPart.filePath.c_str(),
                                        std::ios::out | std::ios::binary);
                                    if (this->_currentPart.file.is_open() == false) {
                                        return HTTP_RES_INTERNAL_SERVER_ERROR;
                                    }

                                    this->_mpState = MP_READING_BODY;
                                    this->_mpCurrentLine = "";
                                    continue;
                                }

                                /* Parse The Header */
                                _generateHeadersMap(line, this->_partLevelHeaders);
                                continue;
                            }

                            if (this->_mpState == MP_READING_BODY) {
                                /* Check if got boundary , if so save the file then store it in the
                                 * vector */
                                if ("--" + this->_boundary == line ||
                                    "--" + this->_boundary + "--" == line) {

                                    if (this->_mpCurrentLine.empty() == false) {
                                        size_t writeLen = this->_mpCurrentLine.size();
                                        // Remove \n and \r from the end of the buffer before
                                        // writing
                                        if (writeLen >= 1 &&
                                            this->_mpCurrentLine[writeLen - 1] == '\n')
                                            writeLen--;
                                        if (writeLen >= 1 &&
                                            this->_mpCurrentLine[writeLen - 1] == '\r')
                                            writeLen--;

                                        this->_currentPart.file.write(this->_mpCurrentLine.c_str(),
                                                                      writeLen);
                                    }
                                    this->_currentPart.file.close();
                                    this->_files.push_back(this->_currentPart);

                                    /* if normal boundary found reset the state machine */
                                    if ("--" + this->_boundary == line) {
                                        this->_mpState = MP_READING_HEADERS;
                                        this->_currentPart = FilePart();
                                        this->_partLevelHeaders.clear();
                                        continue;
                                    }
                                    /* if end boundary found set body complete */
                                    this->_mpState = MP_COMPLETE;
                                    this->_state = BODY_COMPLETE;
                                    return HTTP_RES_OK;
                                }

                                // NOTE: What the hack is this
                                if (this->_mpCurrentLine.empty() == false) {
                                    this->_currentPart.file.write(this->_mpCurrentLine.c_str(),
                                                                  this->_mpCurrentLine.size());
                                }

                                // NOTE: What the hack is this
                                this->_mpCurrentLine = line;
                                if (hadCr == true) {
                                    this->_mpCurrentLine += "\r";
                                }
                                this->_mpCurrentLine += "\n";
                            }
                        }
                        return HTTP_RES_OK;
                    }
                    return HTTP_RES_OK;
                }
            }

            return HTTP_RES_OK;
        }
        // Here is how i have tried to parse the request body in multi part form data, since i store
        // the whole body in a temp file
    } Body;

    parserPhase _phase;
    requestType _requestType;
    RequestURI _requestURI;
    int _requestVersion[2];
    ServerConfig *_config;
    std::map<std::string, std::vector<std::string> > _headers;
    Body _body;
    httpResult _res;

    static std::vector<std::string> _getTokens(std::string line, std::string delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        bool inDoubleQuotes = false;
        bool inSingleQuotes = false;
        size_t tokenStart = 0;
        size_t i = 0;

        while (i < line.length()) {
            char c = line[i];

            if (c == '"' && !inSingleQuotes) {
                inDoubleQuotes = !inDoubleQuotes;
            }
            else if (c == '\'' && !inDoubleQuotes) {
                inSingleQuotes = !inSingleQuotes;
            }

            if (!inDoubleQuotes && !inSingleQuotes) {
                if (line.substr(i, delimiter.length()) == delimiter) {
                    token = line.substr(tokenStart, i - tokenStart);
                    tokens.push_back(token);
                    i += delimiter.length();
                    tokenStart = i;
                    continue;
                }
            }
            i++;
        }

        if (tokenStart < line.length()) {
            tokens.push_back(line.substr(tokenStart));
        }
        return tokens;
    }

    requestType _getRequestTypeFromString(std::string _requestType)
    {
        if (_requestType == "GET")
            return GET;
        if (_requestType == "POST")
            return POST;
        if (_requestType == "DELETE")
            return DELETE;
        if (_requestType == "PUT")
            return PUT;
        if (_requestType == "HEAD")
            return HEAD;
        if (_requestType == "OPTIONS")
            return OPTIONS;
        if (_requestType == "TRACE")
            return TRACE;
        if (_requestType == "CONNECT")
            return CONNECT;
        return UNKNOWN;
    }

    /*
     * Request-Line = Method SP Request-URI SP HTTP-Version CRLF
     * CRLF: "\r\n"
     */
    httpResult _parseRequestLine(std::string requestLine)
    {
        std::string requestVersion;
        std::string requestType;
        std::string requestURI;

        std::vector<std::string> tokens = _getTokens(requestLine, " ");
        if (tokens.size() != 3) {
            return HTTP_RES_BAD_REQUEST;
        }

        requestType = tokens[0];
        requestURI = tokens[1];
        requestVersion = tokens[2];

        if (_isValidToken(requestType) == false) {
            return HTTP_RES_BAD_REQUEST;
        }

        this->_requestType = _getRequestTypeFromString(requestType);
        if (this->_requestType == UNKNOWN) {
            return HTTP_RES_BAD_REQUEST;
        }

        if ((this->_requestType == GET || this->_requestType == POST ||
             this->_requestType == DELETE) == false) {
            return HTTP_RES_NOT_IMPLEMENTED;
        }

        if (this->_requestURI.parseRequestURI(requestURI) != HTTP_RES_OK) {
            return HTTP_RES_BAD_REQUEST;
        }

        /*
         * request version parsing
         * HTTP/<major>.<minor>
         */
        std::vector<std::string> requestVersiongTokens = _getTokens(requestVersion, "/");
        if (requestVersiongTokens.size() != 2) {
            return HTTP_RES_BAD_REQUEST;
        }
        if (requestVersiongTokens[0] != "HTTP") {
            return HTTP_RES_BAD_REQUEST;
        }

        std::vector<std::string> majorMinor = _getTokens(requestVersiongTokens[1], ".");
        if (majorMinor.size() == 1) {
            this->_requestVersion[0] = std::atoi(majorMinor[0].c_str());
            return HTTP_RES_OK;
        }
        if (majorMinor.size() == 2) {
            this->_requestVersion[0] = std::atoi(majorMinor[0].c_str());
            this->_requestVersion[1] = std::atoi(majorMinor[1].c_str());
            return HTTP_RES_OK;
        }
        if (majorMinor.size() != 2) {
            return HTTP_RES_BAD_REQUEST;
        }
        return HTTP_RES_OK;
    }

    /*
     *
     * token = 1*tchar
     *
     * tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*"
     *          / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
     *          / DIGIT / ALPHA
     *          ; any VCHAR, except delimiters
     */
    static bool _isValidToken(const std::string &str)
    {
        if (str.empty()) {
            return false;
        }
        int stringSize = str.size();

        for (int i = 0; i < stringSize; i++) {
            if (std::isalnum(char(str[i]))) {
                // DIGIT or ALPHA
                continue;
            }
            switch (str[i]) {
            case '!':
            case '#':
            case '$':
            case '%':
            case '&':
            case '\'':
            case '*':
            case '+':
            case '-':
            case '.':
            case '^':
            case '_':
            case '`':
            case '|':
            case '~':
                continue;
            default:
                return false;
            }
        }
        return true;
    }

    /*
     * Header-Field = field-name ":" OWS field-value OWS
     * OWS = *( SP / HTAB ) optional whitespace either ' ' or '\t'
     *
     * field-name     = token                         | Paresed
     * field-value    = *( field-content / obs-fold ) | Not Parsed
     *
     * No whitespace is allowed between the header field-name and colon
     *
     *  TODO:
     *  * A server that receives a request header field, or set of fields,
     *  * larger than it wishes to process MUST respond with an appropriate 4xx
     *  * (Client Error) status code.
     *
     *
     *  The Extracted header name is inserted to a map
     *  holding the raw (non parsed) header value
     */
    static httpResult
    _parseRequestHeaderName(std::string headerField,
                            std::map<std::string, std::vector<std::string> > &_headers)
    {

        size_t colonPos = headerField.find_first_of(":");
        if (colonPos == std::string::npos) {
            return HTTP_RES_BAD_REQUEST;
        }

        std::string fieldName = headerField.substr(0, colonPos);
        headerField.erase(0, colonPos + 1);
        if (_isValidToken(fieldName) == false) {
            return HTTP_RES_BAD_REQUEST;
        }

        std::string fieldValue = trim(headerField);

        // if the key doesn't exist
        if (_headers.find(fieldName) == _headers.end()) {
            std::vector<std::string> tmp;
            tmp.push_back(fieldValue);
            _headers.insert(std::pair<std::string, std::vector<std::string> >(fieldName, tmp));
            return HTTP_RES_OK;
        }
        // key already exist
        _headers[fieldName].push_back(fieldValue);
        return HTTP_RES_OK;
    }

    static httpResult
    _generateHeadersMap(std::string headerLines,
                        std::map<std::string, std::vector<std::string> > &_headers)
    {

        std::vector<std::string> headers = _getTokens(headerLines, CRLF);
        if (headers.empty() == true) {
            return HTTP_RES_BAD_REQUEST;
        }

        size_t size = headers.size();
        httpResult res;

        for (int i = 0; i < int(size); i++) {
            res = _parseRequestHeaderName(headers[i], _headers);
            if (res != HTTP_RES_OK) {
                return res;
            }
        }
        return HTTP_RES_OK;
    }

    httpResult _parseRequestHeaderValue()
    {
        for (std::map<std::string, std::vector<std::string> >::iterator it = this->_headers.begin();
             it != this->_headers.end(); ++it) {
            std::vector<std::string> values;
            for (int i = 0; i < int(it->second.size()); i++) {
                std::vector<std::string> tokens = _getTokens(it->second[i], ",");
                for (int j = 0; j < int(tokens.size()); j++) {
                    ;
                    values.push_back(trim(tokens[j]));
                }
            }
            it->second = values;
        }
        return HTTP_RES_OK;
    }

  public:
    static void printHttpResult(httpResult result)
    {
        std::cerr << "HTTP: " << httpResultToString(result) << std::endl;
    }
    static void printHeaders(std::map<std::string, std::vector<std::string> > _headers)
    {
        std::map<std::string, std::vector<std::string> > foo = _headers;

        for (std::map<std::string, std::vector<std::string> >::iterator itMap = foo.begin();
             itMap != foo.end(); ++itMap) {
            std::string key = itMap->first;
            std::vector<std::string> &values = itMap->second;

            std::cout << "Header: " << key << std::endl;

            for (std::vector<std::string>::iterator itVec = values.begin(); itVec != values.end();
                 ++itVec) {
                std::cout << "  Value: " << *itVec << std::endl;
            }
        }
    }
    static void printRaw(const std::string str)
    {
        for (size_t i = 0; i < str.size(); i++) {
            switch (str[i]) {
            case '\r':
                std::cout << "\\r";
                break;
            case '\n':
                std::cout << "\\n";
                break;
            case '\t':
                std::cout << "\\t";
                break;
            case '\0':
                std::cout << "\\0";
                break;
            case '\\':
                std::cout << "\\\\";
                break;
            default:
                if (str[i] < 32 || str[i] == 127) {
                    // Non-printable character → show hex value
                    std::cout << "\\x"
                              << "0123456789abcdef"[(str[i] >> 4) & 0x0F]
                              << "0123456789abcdef"[str[i] & 0x0F];
                }
                else {
                    std::cout << str[i];
                }
                break;
            }
        }
        std::cout << std::endl;
    }
};

#endif // !__REQUEST_HPP__
