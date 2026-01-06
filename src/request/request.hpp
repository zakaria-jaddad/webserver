#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
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
 *
 * TODO: Refactor Field Line Parser
 *
 */
class HttpParser {

  public:
    HttpParser() : __state(REQUEST_LINE)
    {
        this->_requestType = "";
        this->_requestURI = "";
        this->_requestVersion = "";
        this->_type = UNKNOWN;
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

        if (this->__state == REQUEST_LINE) {

            std::string requestLine = rawData.substr(0, rawData.find(CRLF));
            rawData.erase(0, requestLine.size() + strlen(CRLF));

            /* parse Request-Line ... */
            res = _parseRequestLine(requestLine);
            if (res != HTTP_RES_OK)
                return res;

            this->__state = HEADERS;
        }

        if (this->__state == HEADERS) {

            std::string headerLines;

            size_t doubleCrlfPos = rawData.find(DOUBLE_CRLF);
            if (doubleCrlfPos != std::string::npos) {
                headerLines = rawData.substr(0, doubleCrlfPos);
                rawData.erase(0, headerLines.size() + strlen(DOUBLE_CRLF));
                this->__state = BODY;
            }

            res = _parseRequestHeaders(headerLines);
            if (res != HTTP_RES_OK)
                return res;
        }

        if (this->__state == BODY) {

            /*
             * TODO:
             * Parse Request Body
             */

            /* return HTTP_RES_OK */
        }

        return HTTP_RES_OK;
    };

    requestType getType() const { return this->_type; }
    std::string getRequestType() const { return this->_requestType; }
    std::string getRequestURI() const { return this->_requestURI; }
    std::string getRequestVersion() const { return this->_requestVersion; }
    std::map<std::string, std::vector<std::string> > getHeaders() const { return this->_headers; }

  private:
    typedef enum {
        REQUEST_LINE,
        HEADERS,
        BODY,
    } parserState;

  private:
    parserState __state;
    requestType _type;
    std::string _requestType; /* _requestType = token */
    std::string _requestURI;
    std::string _requestVersion;
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

        std::vector<std::string> tokens = _getTokens(requestLine, " ");
        if (tokens.size() != 3) {
            return HTTP_RES_BAD_REQUEST;
        }

        this->_requestType = tokens[0];
        this->_requestURI = tokens[1];
        this->_requestVersion = tokens[2];
        this->_type = _getRequestTypeFromString(this->_requestType);
        if (this->_type == UNKNOWN) {
            return HTTP_RES_NOT_IMPLEMENTED;
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
