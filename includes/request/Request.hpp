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
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define WHITE "\033[0m"

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
    HttpParser(void);
    HttpParser(ServerConfig *ServerConfig);
    ~HttpParser();

  public:
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
    static requestType getRequestTypeFromString(std::string _requestType);

    typedef enum {
        REQUEST_LINE,
        HEADERS,
        BODY,
        COMPLETE,
    } parserPhase;

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
    static const char *httpResultToString(httpResult result);

    /*
     * FilePart Holds file information
     * case1: file exist body content will be stored in the file.
     * case2: file don't exist since it's a simpe form data content would be in the buffer
     * */
    struct FilePart {
        std::string name;     // Form field name (e.g., "avatar")
        std::string filename; // Original filename (e.g., "me.png")
        std::string contentType;
        std::string filePath; // Path where the final file is saved on server
        std::fstream file;
        std::string buffer;

        void printInfo() const;

        // Default constructor
        FilePart();
        FilePart(const FilePart &other);
        FilePart &operator=(const FilePart &other);

        // Destructor
        ~FilePart();
    };
    /* ---- GETTERS ----*/
    requestType getRequestType() const;
    const int *getRequestVersion() const;
    std::map<std::string, std::vector<std::string> > getHeaders() const;
    httpResult getRes() const;

    std::string getRequestUriPath() const;
    std::string getRequestUriRawPath() const;
    std::string getRequestUriRawParams() const;
    std::map<std::string, std::string> getRequestUriParams() const;
    ServerConfig *getServerConfig() const;
    const std::map<std::string, std::vector<FilePart> > &getBodyFormFields() const;

    /* ---- SETTERS ----*/
    void setRes(httpResult res);

    bool isComplete();
    bool isBodyComplete();

    bool isProcessingBody();

    httpResult parseHttpRequest(std::string rawData);

  private:
    static char _hexToChar(std::string hex);
    static bool _isHex(char c);
    static bool _isValidPercentEncoding(std::string str);
    static std::string _urlDecode(std::string str);
    static void _replacePlusWithSpace(std::string &str);
    static void _parseQueryString(std::string &query, std::map<std::string, std::string> &store);

    /*
     *
     * token = 1*tchar
     *
     * tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*"
     *          / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
     *          / DIGIT / ALPHA
     *          ; any VCHAR, except delimiters
     */
    static bool _isValidToken(const std::string &str);
    static std::vector<std::string> _getTokens(std::string line, std::string delimiter);

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

        typedef enum {
            CHUNK_READ_SIZE,
            CHUNK_READ_DATA,
            CHUNK_READ_CRLF,
            CHUNK_DONE,
        } chunkState;

        static std::string getBodyStateString(bodyState state);
        static std::string getMultiPartStateString(multiPartState state);

      private:
        void _normalizePath(void);
        std::string _resolveFilePath(std::string &upload_store);
        httpResult _initParsingInfo();
        std::string extractPar(const std::string header, std::string key);
        void _fileCleanUp();

      private:
        /* Content Type Info */
        std::string _boundary;
        std::string _mediaType;
        multiPartState _mpState;

        // For uploaded files
        FilePart _currentPart;
        std::map<std::string, std::vector<std::string> > _partLevelHeaders;
        std::string _mpCurrentLine;

        /* Chunk Encoding */
        chunkState _chunkState;
        std::string _chunkBuffer;

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
        std::map<std::string, std::vector<FilePart> > _formFields;

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

        Body();
        ~Body();
        void init(HttpParser *httpParser);
        httpResult parseRequestBody(std::string rawData);
    } Body;

  private:
    /* HttpParser private attributes */
    parserPhase _phase;
    requestType _requestType;

    typedef struct RequestURI {
      public:
        std::string path;
        std::map<std::string, std::string> params;
        std::string rawURI;
        std::string rawParams;

        std::string getPath() const;
        std::map<std::string, std::string> getParams() const;
        std::string getRawUri() const;
        std::string getRawParams() const;

        HttpParser::httpResult parseRequestURI(std::string uri);
    } RequestURI;
    RequestURI _requestURI;

    int _requestVersion[2];
    ServerConfig *_config;
    std::map<std::string, std::vector<std::string> > _headers;
    Body _body;
    httpResult _res;
    std::map<std::string, std::string> _mimeToExt;
    std::string _requestLineBuffer;
    std::string _headerBuffer;

    /*
     * Request-Line = Method SP Request-URI SP HTTP-Version CRLF
     * CRLF: "\r\n"
     */
    httpResult _parseRequestLine(std::string requestLine);

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
                            std::map<std::string, std::vector<std::string> > &_headers);
    static httpResult
    _generateHeadersMap(std::string headerLines,
                        std::map<std::string, std::vector<std::string> > &_headers);

    httpResult _parseRequestHeaderValue();

  public:
    static void printHttpResult(httpResult result);
    static void printHeaders(std::map<std::string, std::vector<std::string> > _headers);
    static void printRaw(const std::string str);
};

#endif // !__REQUEST_HPP__
