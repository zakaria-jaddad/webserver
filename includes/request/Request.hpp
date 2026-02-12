#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include "../config/Config.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#define CRLF "\r\n"
#define DOUBLE_CRLF "\r\n\r\n"

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
    HttpParser(void) : _phase(REQUEST_LINE)
    {
        this->_requestURI.path = "";
        this->_requestURI.params.clear();
        this->_requestURI.rawPath = "";
        this->_requestVersion[0] = 0;
        this->_requestVersion[1] = 0;
        this->_requestType = UNKNOWN;
        this->_config = NULL; // tmp
    }

    HttpParser(const Config *config) : _phase(REQUEST_LINE)
    {
        this->_requestURI.path = "";
        this->_requestURI.params.clear();
        this->_requestURI.rawPath = "";
        this->_requestVersion[0] = 0;
        this->_requestVersion[1] = 0;
        this->_requestType = UNKNOWN;
        this->_config = config;
    }

    ~HttpParser() {}

    /* Server Errors */
    typedef enum {
        HTTP_VERSION_NOT_SUPPORTED = 505,
        HTTP_RES_OK = 200,
        HTTP_RES_INTERNAL_SERVER_ERROR = 500,
        HTTP_RES_BAD_REQUEST = 400,
        HTTP_RES_NOT_IMPLEMENTED = 501,
        HTTP_REST_METHOD_NOT_ALLOWED = 405,
    } httpResult;

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
            }

            res = _generateHeadersMap(headerLines);
            if (res != HTTP_RES_OK) {
                return res;
            }
        }

        if (this->_phase == BODY) {

            /*
             * NOTE: Header fields get's parsed usually after the message's entire header section
             * has been processed
             */

            res = _parseRequestHeaderValue();
            if (res != HTTP_RES_OK) {
                return res;
            }

            /*
             * TODO:
             * Parse Request Body
             */
            this->_phase = COMPLETE;
            /* return HTTP_RES_OK */
        }

        return HTTP_RES_OK;
    };

    requestType getRequestType() const { return this->_requestType; }
    std::string getRequestURIPath() const { return this->_requestURI.getPath(); }
    const int *getRequestVersion() const { return this->_requestVersion; }
    std::map<std::string, std::vector<std::string> > getHeaders() const { return this->_headers; }
    std::map<std::string, std::string> getRequestURIParams() const
    {
        return this->_requestURI.getParams();
    }
    std::string getRequestRawPath() const { return this->_requestURI.getRawPath(); }

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
        std::string rawPath;
        std::string getPath() const { return this->path; }
        std::map<std::string, std::string> getParams() const { return this->params; }
        std::string getRawPath() const { return this->rawPath; }

        HttpParser::httpResult parseRequestURI(std::string uri)
        {
            this->rawPath = uri;
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
    } requestURI;

    typedef enum {
        REQUEST_LINE,
        HEADERS,
        BODY,
        COMPLETE,
    } parserPhase;

  private:
    parserPhase _phase;
    requestType _requestType;
    requestURI _requestURI;
    int _requestVersion[2];
    const Config *_config;
    // std::map<std::string, std::string> __headers;
    std::map<std::string, std::vector<std::string> > _headers;

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
            this->_headers.insert(std::pair<std::string, std::vector<std::string> >(
                fieldName, tmp));
            return HTTP_RES_OK;
        }
        // key already exist
        this->_headers[fieldName].push_back(fieldValue);
        return HTTP_RES_OK;
    }

    /*
     * NOTE:
     * * The contents within a given field value are not parsed until a later stage of message
     * interpretation
     * * usually after the message's entire header section has been processed
     *
     */
public:
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
                    values.push_back(trim(tokens[j]));
                }
            }
            it->second = values;
        }
        return HTTP_RES_OK;
    }
};

#endif // !__REQUEST_HPP__
