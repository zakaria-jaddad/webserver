#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <string>

#define CRLF "\r\n"
#define DOUBLE_CRLF "\r\n\r\n"

//requestURI struct
    typedef struct s_requestURI {
	    std::string path;
	    std::map<std::string, std::string> params;
    } t_reqURI;
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
 *
 * TODO: Refactor Field Line Parser
 *
 */
class HttpParser {

  public:
    HttpParser() : _phase(REQUEST_LINE)
    {
        this->_requestURI.path = "";
        this->_requestVersion[0] = 0;
        this->_requestVersion[1] = 0;
        this->_requestType = UNKNOWN;
    }

    ~HttpParser() {}

    /* Server Errors */
    typedef enum {
        HTTP_VERSION_NOT_SUPPORTED = 505,
        HTTP_RES_OK = 200,
        HTTP_RES_INTERNAL_SERVER_ERROR = 500,
        HTTP_RES_BAD_REQUEST = 400,
        HTTP_RES_NOT_IMPLEMENTED = 501,
    } httpResult;

    typedef enum {
        GET,
        POST,
        DELETE,
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
            if (res != HTTP_RES_OK)
                return res;

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

            res = _parseRequestHeaders(headerLines);
            if (res != HTTP_RES_OK) {
                return res;
            }
        }

        if (this->_phase == BODY) {

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
    std::string getRequestURIPath() const { return this->_requestURI.path; }
    std::map<std::string, std::string> getRequestURIParams() const { return this->_requestURI.params; }
    const int *getRequestVersion() const { return this->_requestVersion; }
    std::map<std::string, std::vector<std::string> > getHeaders() const { return this->_headers; }

    bool isComplete() { return this->_phase == HttpParser::COMPLETE; }
    int parseRequestURI(t_reqURI& req, std::string uri);

  private:
    typedef enum {
        REQUEST_LINE,
        HEADERS,
        BODY,
        COMPLETE,
    } parserPhase;

  private:
    parserPhase _phase;
    requestType _requestType;
    t_reqURI _requestURI;
    int _requestVersion[2];
    std::map<std::string, std::string> __headers;
    std::map<std::string, std::vector<std::string> > _headers;


    std::vector<std::string> _getTokens(std::string line, std::string delimiter)
    {
        size_t pos = 0;
        std::vector<std::string> tokens;
        std::string token;
        while ((pos = line.find(delimiter)) != std::string::npos) {
            token = line.substr(0, pos);
            tokens.push_back(token);
            line.erase(0, pos + delimiter.length());
        }
        if (line.empty() == false)
            tokens.push_back(line);
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
        if (_isValidToken(requestType) == false) {
            return HTTP_RES_BAD_REQUEST;
        }

        requestURI = tokens[1];
        requestVersion = tokens[2];

        this->_requestType = _getRequestTypeFromString(requestType);
        if (this->_requestType == UNKNOWN) {
            return HTTP_RES_NOT_IMPLEMENTED;
        }

        if (parseRequestURI(this->_requestURI, requestURI)) {
            return HTTP_RES_BAD_REQUEST;
        }

        /*
         * request versiong parsing
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
     */
    httpResult _parseRequestHeader(std::string headerField)
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

        this->__headers.insert(std::pair<std::string, std::string>(fieldName, fieldValue));
        return HTTP_RES_OK;
    }

    /*
     * NOTE:
     * * The contents within a given field value are not parsed until a later stage of message
     * interpretation
     * * usually after the message's entire header section has been processed
     */
    httpResult _parseRequestHeaders(std::string headerLines)
    {

        std::vector<std::string> headers = _getTokens(headerLines, CRLF);
        if (headers.empty() == true) {
            return HTTP_RES_BAD_REQUEST;
        }

        size_t size = headers.size();
        httpResult res;

        for (int i = 0; i < int(size); i++) {
            res = _parseRequestHeader(headers[i]);
            if (res != HTTP_RES_OK) {
                return res;
            }
        }
        return HTTP_RES_OK;
    }
};

#endif // !__REQUEST_HPP__
