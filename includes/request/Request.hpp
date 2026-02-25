#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include "../config/Config.hpp"
#include "../response/ResolvedConfig.hpp"
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
#include <unistd.h>
#include <vector>

#define CRLF "\r\n"
#define DOUBLE_CRLF "\r\n\r\n"

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

    const char *httpResultToString(httpResult result)
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
                headerLines = rawData.substr(0, doubleCrlfPos + 2);
                rawData.erase(0, headerLines.size() + strlen(DOUBLE_CRLF));
                this->_phase = BODY;
                // BUG:
                this->_body.init(this);
            }

            res = _generateHeadersMap(headerLines);
            if (res != HTTP_RES_OK) {
                return res;
            }
        }

        if (this->_phase == BODY) {

            /* If request type other than POST ignore the body */
            if (this->_requestType != POST) {
                this->_phase = COMPLETE;
                return HTTP_RES_OK;
            }

            if (this->_body._state == Body::BODY_NOT_STARTED) {

                res = _parseRequestHeaderValue();
                if (res != HTTP_RES_OK) {
                    return res;
                }
            }

            res = this->_body.parseRequestBody(rawData);
            if (res != HTTP_RES_OK) {
                return res;
            }

            // WARNING: This should be removed from here
            this->_phase = COMPLETE;
        }

        return HTTP_RES_OK;
    };

    requestType getRequestType() const { return this->_requestType; }
    const int *getRequestVersion() const { return this->_requestVersion; }
    std::map<std::string, std::vector<std::string> > getHeaders() const { return this->_headers; }

    //
    std::string getRequestUriPath() const { return this->_requestURI.getPath(); }
    std::string getRequestUriRawPath() const { return this->_requestURI.getRawUri(); }
    std::string getRequestUriRawParams() const { return this->_requestURI.getRawParams(); }
    std::map<std::string, std::string> getRequestUriParams() const
    {
        return this->_requestURI.getParams();
    }
    ServerConfig *getServerConfig() const { return this->_config; }

    bool isComplete() { return this->_phase == HttpParser::COMPLETE; }

  private:
    typedef struct RequestURI {
      private:
        char hexToChar(std::string hex)
        {
            int ret;
            std::stringstream ss;
            ss << std::hex << hex;
            ss >> ret;
            return (ret);
        }

        bool isHex(char c)
        {
            return (!(c < '0' || (c > '9' && c < 'A') || (c > 'F' && c < 'a') || c > 'f'));
        }

        bool isValidPercentEncoding(std::string str)
        {
            for (size_t i = 0; i < str.length(); i++) {
                if (i > 1 || !isHex(str[i]))
                    return (0);
            }
            return (1);
        }

        std::string urlDecode(std::string &str)
        {
            std::string dec = "";

            for (size_t i = 0; i < str.length(); i++) {
                if (str[i] == '%' && isValidPercentEncoding(str.substr(i + 1, 2))) {
                    dec += hexToChar(str.substr(i + 1, 2));
                    i += 2;
                }
                else if (str[i] == '%') {
                    return (""); // error
                }
                else
                    dec += str[i];
            }
            return (dec);
        }

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
            // find first accurance of ?
            size_t pos = uri.find_first_of("?");
            if (pos != std::string::npos) {
                this->rawParams = uri.substr(pos + 1);
            }
            std::stringstream ss(uri);
            std::string param;
            std::string key;
            std::string val;

            getline(ss, this->path, '?');
            while (getline(ss, param, '&')) {
                key = "";
                val = "";
                std::stringstream paramStream(param);
                getline(paramStream, key, '=');
                paramStream >> val;
                this->params[key] = val;
            }
            this->path = urlDecode(this->path);
            if (this->path.size() == 0) {
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
            BODY_COMPLETE,
        } bodyState;

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

            file.open(this->filePath.c_str(), ios::out | ios::trunc);
            if (file.is_open() == false) {
                cerr << "Error: " << this->filePath << " Failed to open file" << endl;
                return HTTP_RES_INTERNAL_SERVER_ERROR;
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

      public:
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
        }

        ~Body()
        {
            this->file.close();
            delete this->conf;
        }

        void init(HttpParser *httpParser)
        {
            this->httpParser = httpParser;
            // cout << "=== " << this->httpParser->_config->getSize() << " ===" << endl;
            this->_maxBodySize = this->httpParser->_config->getSize();

            this->conf = new ResolvedConfig(httpParser->getRequestUriPath(), httpParser->_config);
            normalizePath();

            // std::cout << "========= " << this->normalizedPath << " ========" << std::endl;
            // std::cout << "========= " << httpParser->getRequestUriPath()
            //           << " ========" << std::endl;
        }

        bodyType _type;
        bodyState _state;
        size_t _maxBodySize;
        size_t _contentLength;
        size_t _bytesReceived;
        std::string _body;
        HttpParser *httpParser;
        ResolvedConfig *conf;
        std::string normalizedPath;
        fstream file;
        string filePath;
        // long currentChunkSize;
        // long currentChunkRead;
        // bool lastChunkReceived;

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

            size_t rawDataSize = rawData.size();
            if (this->_state == BODY_READING) {
                // This part should be removed
                if (this->_type == CONTENT_LENGTH) {
                    // ----
                    if (rawDataSize + this->_bytesReceived > this->_contentLength) {
                        file.close();
                        if (remove(this->filePath.c_str()) != 0) {
                            std::cerr << "Error: " << this->filePath << " deleting file.\n";
                            return HTTP_RES_INTERNAL_SERVER_ERROR;
                        }
                        return HTTP_RES_REQUEST_ENTITY_TOO_LARGE;
                    }
                    file.write(rawData.c_str(), rawDataSize);
                    this->_bytesReceived = this->_bytesReceived + rawDataSize;
                    if (this->_bytesReceived == this->_contentLength) {
                        this->_state = BODY_COMPLETE;
                    }
                    return HTTP_RES_OK;
                }
            }

            /* NOTE: Error When More Body Content Arrives */
            if (this->_state == BODY_COMPLETE) {
                file.close();
                if (remove(this->filePath.c_str()) != 0) {
                    std::cerr << "Error: " << this->filePath << " deleting file.\n";
                    return HTTP_RES_INTERNAL_SERVER_ERROR;
                }
                return HTTP_RES_REQUEST_ENTITY_TOO_LARGE;
            }
            return HTTP_RES_OK;
        }
    } Body;

    parserPhase _phase;
    requestType _requestType;
    RequestURI _requestURI;
    int _requestVersion[2];
    ServerConfig *_config;
    std::map<std::string, std::vector<std::string> > _headers;
    Body _body;

    std::vector<std::string> _getTokens(std::string line, std::string delimiter)
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
    bool _isValidToken(const std::string &str)
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
    httpResult _parseRequestHeaderName(std::string headerField)
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
        if (this->_headers.find(fieldName) == this->_headers.end()) {
            std::vector<std::string> tmp;
            tmp.push_back(fieldValue);
            this->_headers.insert(
                std::pair<std::string, std::vector<std::string> >(fieldName, tmp));
            return HTTP_RES_OK;
        }
        // key already exist
        this->_headers[fieldName].push_back(fieldValue);
        return HTTP_RES_OK;
    }

    httpResult _generateHeadersMap(std::string headerLines)

    {

        std::vector<std::string> headers = _getTokens(headerLines, CRLF);
        if (headers.empty() == true) {
            return HTTP_RES_BAD_REQUEST;
        }

        size_t size = headers.size();
        httpResult res;

        for (int i = 0; i < int(size); i++) {
            res = _parseRequestHeaderName(headers[i]);
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
    void printHttpResult(httpResult result)
    {
        std::cerr << "HTTP: " << httpResultToString(result) << std::endl;
    }
    void printHeaders()
    {
        std::map<std::string, std::vector<std::string> > foo = this->_headers;

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
};

#endif // !__REQUEST_HPP__
