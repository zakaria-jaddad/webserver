#include "../../includes/request/Request.hpp"
#include <cstddef>
#include <ios>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

/* ===================================================================================== */
/* ------------------------------ CONSTRUCTOR DESTRUCTOR ------------------------------- */
/* ===================================================================================== */

HttpParser::HttpParser(std::vector<ServerConfig *> &servers)
    : _phase(REQUEST_LINE), _servers(servers), _body()
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
    this->_requestLineBuffer = "";
    this->_headerBuffer = "";
    this->_pendingData = "";
    this->_mimeToExt["image/png"] = ".png";
    this->_mimeToExt["image/jpeg"] = ".jpg";
    this->_mimeToExt["image/gif"] = ".gif";
    this->_mimeToExt["text/plain"] = ".txt";
    this->_mimeToExt["text/html"] = ".html";
    this->_mimeToExt["application/json"] = ".json";
    this->_mimeToExt["application/pdf"] = ".pdf";
    this->_mimeToExt["application/octet-stream"] = ".bin";
}

HttpParser::~HttpParser() {}

/* ---- GETTERS ----*/
HttpParser::requestType HttpParser::getRequestType() const { return this->_requestType; }

const int *HttpParser::getRequestVersion() const { return this->_requestVersion; }

const std::map<std::string, std::vector<std::string> > &HttpParser::getHeaders() const
{
    return this->_headers;
}

HttpParser::httpResult HttpParser::getRes() const { return this->_res; }

std::string HttpParser::getRequestUriPath() const { return this->_requestURI.getPath(); }

std::string HttpParser::getRequestUriRawPath() const { return this->_requestURI.getRawUri(); }

std::string HttpParser::getRequestUriRawParams() const { return this->_requestURI.getRawParams(); }

std::map<std::string, std::string> HttpParser::getRequestUriParams() const
{
    return this->_requestURI.getParams();
}

ServerConfig *HttpParser::getServerConfig() const { return this->_config; }

const std::map<std::string, std::vector<HttpParser::FilePart> > &
HttpParser::getBodyFormFields() const
{
    return this->_body._formFields;
}

std::string HttpParser::getPendingData() const { return this->_pendingData; }

std::string HttpParser::getRequestRawBodyFilePath() const { return this->_body.filePath; }

size_t HttpParser::getBodySize() const { return this->_body._bytesReceived; }

std::string HttpParser::getHostHeader() { return this->_headers["Host"][0]; }

/* ---- SETTERS ----*/

void HttpParser::setRes(httpResult res) { this->_res = res; }

bool HttpParser::isComplete() { return this->_phase == HttpParser::COMPLETE; }
bool HttpParser::isBodyComplete() { return this->_body._state == Body::BODY_COMPLETE; }

bool HttpParser::isProcessingBody()
{
    return this->_phase == HttpParser::BODY && this->_body._state == Body::BODY_PROCESSING;
}

bool HttpParser::keepAlive() const
{
    std::map<std::string, std::vector<std::string> >::const_iterator it;
    std::string connection;

    it = this->_headers.find("Connection");
    if (it != this->_headers.end() && !it->second.empty()) {
        connection = it->second[0];
        for (size_t i = 0; i < connection.size(); ++i)
            connection[i] = static_cast<char>(tolower(connection[i]));
    }

    // HTTP/1.1 defaults to keep-alive; HTTP/1.0 (and unknown) default to close.
    if (this->_requestVersion[0] == 1 && this->_requestVersion[1] == 1) {
        if (connection == "close")
            return false;
        return true;
    }

    if (connection == "keep-alive")
        return true;
    return false;
}

/* ===================================================================================== */
/* ------------------------------ PUBLIC iNTERFACE ------------------------------------- */
/* ===================================================================================== */

HttpParser::httpResult HttpParser::parseHttpRequest(std::string rawData)
{

    httpResult res;

    // Preapending leftover data to rawData
    rawData = this->_pendingData + rawData;
    this->_pendingData.clear();

    if (this->_phase == REQUEST_LINE) {

        this->_requestLineBuffer += rawData;

        size_t crlfPos = this->_requestLineBuffer.find(CRLF);
        if (crlfPos == std::string::npos) {
            // Need More Data
            return HTTP_RES_OK;
        }

        std::string requestLine = this->_requestLineBuffer.substr(0, crlfPos);

        rawData = this->_requestLineBuffer.substr(crlfPos + strlen(CRLF));

        this->_requestLineBuffer.clear();

        /* parse Request-Line ... */
        res = _parseRequestLine(requestLine);

        if (res != HTTP_RES_OK) {
            return res;
        }

        this->_phase = HEADERS;
    }

    if (this->_phase == HEADERS) {

        this->_headerBuffer += rawData;

        size_t doubleCrlfPos = this->_headerBuffer.find(DOUBLE_CRLF);

        if (doubleCrlfPos == std::string::npos) {
            // Need more Data
            return HTTP_RES_OK;
        }

        std::string headerLines = this->_headerBuffer.substr(0, doubleCrlfPos);

        rawData = this->_headerBuffer.substr(doubleCrlfPos + strlen(DOUBLE_CRLF));

        this->_headerBuffer.clear();

        res = _generateHeadersMap(headerLines, this->_headers);
        if (res != HTTP_RES_OK) {
            return res;
        }
        this->_phase = BODY;
    }

    if (this->_phase == BODY) {

        if (this->_body._state == Body::BODY_NOT_STARTED) {

            std::cout << "Start Parsing Headers" << std::endl;
            res = _parseRequestHeaderValue();

            // Verify Host Header
            std::vector<std::string> hostHeader = this->_headers["Host"];
            if (hostHeader.size() != 1) {
                return HTTP_RES_BAD_REQUEST;
            }
            std::cout << "Finished Parsing Headers" << std::endl;
            if (res != HTTP_RES_OK) {
                return res;
            }

            // yhossni trying some bs
            if (this->_config == NULL)
                this->_config = resolveServer(this->_servers, hostHeader[0]);

            ResolvedConfig resolvedConfig(this->getRequestUriPath(), this->_config);
            if (isMethodAllowed(resolvedConfig.getAcceptedMethods(), this->getRequestType()) ==
                false) {
                return HTTP_RES_METHOD_NOT_ALLOWED;
            }

            /* If request type other than POST ignore the body */
            if (this->_requestType != POST) {
                // Save pending Data if pipelined request
                setPendingData(rawData);
                this->_phase = COMPLETE;
                return HTTP_RES_OK;
            }

            this->_body.init(this);
        }

        res = this->_body.parseRequestBody(rawData);
        if (res != HTTP_RES_OK) {
            return res;
        }

        if (this->_body._state == Body::BODY_COMPLETE) {
            // Body is done save remaining rawData for the next request
            this->_phase = COMPLETE;
        }
    }

    return HTTP_RES_OK;
};

/* ===================================================================================== */
/* ------------------------------ NESTED HttpParser CLASS RequestURI ------------------- */
/* ===================================================================================== */

std::string HttpParser::RequestURI::getPath() const { return this->path; }

std::map<std::string, std::string> HttpParser::RequestURI::getParams() const
{
    return this->params;
}
std::string HttpParser::RequestURI::getRawUri() const { return this->rawURI; }
std::string HttpParser::RequestURI::getRawParams() const { return this->rawParams; }

HttpParser::httpResult HttpParser::RequestURI::parseRequestURI(std::string uri)
{
    this->rawURI = uri;
    this->params.clear();

    size_t pos = uri.find("?");

    if (pos != std::string::npos) {
        // Split URI into Path and Query Params
        this->path = uri.substr(0, pos);
        this->rawParams = uri.substr(pos + 1);

        HttpParser::_parseQueryString(this->rawParams, this->params);
    }
    else {
        this->path = uri;
        this->rawParams = "";
    }

    this->path = HttpParser::_urlDecode(this->path);

    if (this->path.empty()) {
        return HttpParser::HTTP_RES_BAD_REQUEST;
    }
    return HttpParser::HTTP_RES_OK;
}

/* ===================================================================================== */
/* -------------------------- NESTED HttpParser CLASS Body ----------------------------- */
/* ===================================================================================== */

HttpParser::Body::Body()
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
    this->_mpState = MP_EXPECTING_BOUNDARY;
    this->_chunkState = CHUNK_READ_SIZE;
    this->_chunkBuffer = "";
}

HttpParser::Body::~Body()
{
    this->file.close();
    delete this->conf;
}

std::string HttpParser::Body::getBodyStateString(bodyState state)
{
    switch (state) {
    case BODY_NOT_STARTED:
        return "BODY_NOT_STARTED";
    case BODY_READING:
        return "BODY_READING";
    case BODY_PROCESSING:
        return "BODY_PROCESSING";
    case BODY_COMPLETE:
        return "BODY_COMPLETE";
    default:
        return "UNKNOWN_BODY_STATE";
    }
}

std::string HttpParser::Body::getMultiPartStateString(multiPartState state)
{
    switch (state) {
    case MP_EXPECTING_BOUNDARY:
        return "MP_EXPECTING_BOUNDARY";
    case MP_READING_HEADERS:
        return "MP_READING_HEADERS";
    case MP_READING_BODY:
        return "MP_READING_BODY";
    case MP_COMPLETE:
        return "MP_COMPLETE";
    default:
        return "UNKNOWN_MULTIPART_STATE";
    }
}

HttpParser::httpResult HttpParser::Body::parseRequestBody(std::string rawData)
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
        std::cout << YELLOW << "Processing Body" << WHITE << std::endl;

        // Content-Length Body Parsing
        if (this->_type == CONTENT_LENGTH) {
            std::cout << YELLOW << "Reading body using Content-Length" << WHITE << std::endl;
            size_t byteRemaining = this->_contentLength - this->_bytesReceived;

            if (rawDataSize > byteRemaining) {
                this->file.write(rawData.c_str(), byteRemaining);
                this->_bytesReceived += byteRemaining;

                // Save leftover rawData for next request if pipelined
                httpParser->_pendingData = rawData.substr(byteRemaining);

                std::cout << GREEN << "Finished Reading body using Content-Length" << WHITE
                          << std::endl;

                this->_state = BODY_PROCESSING;
                this->file.flush();
                this->file.seekg(0, std::ios::beg);
            }
            else {

                file.write(rawData.c_str(), rawDataSize);
                this->_bytesReceived = this->_bytesReceived + rawDataSize;
                if (this->_bytesReceived == this->_contentLength) {

                    std::cout << GREEN << "Finished Reading body using Content-Length" << WHITE
                              << std::endl;
                    this->_state = BODY_PROCESSING;
                    this->file.flush();
                    this->file.seekg(0, std::ios::beg);
                }
            }
        }

        // Chunked Body Parsing
        if (this->_type == CHUNKED) {

            std::cout << YELLOW << "Reading body using Chunked Encoding" << WHITE << std::endl;

            this->_chunkBuffer += rawData;
            std::string &data = this->_chunkBuffer;

            while (data.empty() == false) {

                switch (this->_chunkState) {

                case CHUNK_READ_SIZE: {
                    // get current chun size
                    size_t crlfPos = data.find(CRLF);
                    if (crlfPos != std::string::npos) {
                        std::string encodedChunkSize = data.substr(0, crlfPos);
                        data.erase(0, encodedChunkSize.size() + strlen(CRLF));
                        long chunkSize = hex_to_decimal(this->rawChunkSize + encodedChunkSize);
                        this->rawChunkSize = "";

                        if (chunkSize < 0) {
                            return HTTP_RES_BAD_REQUEST;
                        }
                        if (chunkSize == 0) {
                            this->_chunkState = CHUNK_DONE;
                        }
                        else {
                            this->_chunkState = CHUNK_READ_DATA;
                        }
                        this->currentChunkSize = chunkSize;

                        if (this->_bytesReceived + this->currentChunkSize > this->_maxBodySize) {
                            this->_fileCleanUp();
                            return HTTP_RES_REQUEST_ENTITY_TOO_LARGE;
                        }
                        this->_bytesReceived += this->currentChunkSize;
                    }
                    else {
                        this->rawChunkSize += data;
                        data.clear();
                        return HTTP_RES_OK;
                    }
                    break;
                }

                case CHUNK_READ_DATA: {

                    long byteRemaining = this->currentChunkSize - this->currentChunkRead;

                    long bytesAvailable = data.size();

                    long bytesToTake = std::min(byteRemaining, bytesAvailable);

                    this->file.write(data.c_str(), bytesToTake);

                    this->currentChunkRead += bytesToTake;

                    data.erase(0, bytesToTake);

                    if (this->currentChunkRead == this->currentChunkSize) {
                        this->_chunkState = CHUNK_READ_CRLF;

                        this->currentChunkRead = 0;
                    }
                    else {
                        // Need more data for this chunk
                        // keep leftovers in _chunkBuffer for next time
                        return HTTP_RES_OK;
                    }
                    break;
                }

                case CHUNK_READ_CRLF: {

                    if (data.size() < 2) {
                        // again i need more data keep leftovers and return
                        return HTTP_RES_OK;
                    }
                    if (data.substr(0, 2) != CRLF) {
                        this->_fileCleanUp();
                        return HTTP_RES_BAD_REQUEST;
                    }
                    data.erase(0, 2);
                    this->_chunkState = CHUNK_READ_SIZE;
                    break;
                }
                case CHUNK_DONE: {

                    size_t crlfPos = data.find(CRLF);

                    if (crlfPos == std::string::npos) {
                        return HTTP_RES_OK;
                    }

                    std::string line = data.substr(0, crlfPos);
                    data.erase(0, crlfPos + strlen(CRLF));

                    if (line.empty() == true) {
                        // consumed the CRLF trailler
                        this->file.flush();
                        this->file.seekg(0, std::ios::beg);
                        std::cout << GREEN << "Finished Chunked Body" << WHITE << std::endl;

                        this->_state = BODY_PROCESSING;
                        this->_chunkBuffer.clear();
                        // No need to call in my case this->file.clear();

                        // Save leftover data for the next pipelined request
                        httpParser->_pendingData = data;
                    }
                    break;
                }
                }
            }
        }
    }
    if (this->_state == BODY_PROCESSING) {

        /* NOTE: When POST request rout is a script, body shouldn't be parsed */
        if (isCgi(httpParser->getRequestUriRawPath(), this->conf->getCgi())) {
            this->_state = BODY_COMPLETE;
            this->file.close();
            return HTTP_RES_OK;
        }

        if (this->_mediaType == "application/x-www-form-urlencoded") {
            std::map<std::string, std::string> params;

            std::string urlEncodedForm;
            std::stringstream ss;

            this->file.seekg(0, std::ios::beg);
            ss << this->file.rdbuf();

            urlEncodedForm = ss.str();

            _parseQueryString(urlEncodedForm, params);
            if (params.size() == 0) {
                return HTTP_RES_BAD_REQUEST;
            }

            std::cout << "Url decode form" << std::endl;
            for (std::map<std::string, std::string>::const_iterator it = params.begin();
                 it != params.end(); it++) {
                std::cout << "key: " << it->first << ", value: " << it->second << std::endl;
            }

            _fileCleanUp();
            std::cout << "\033[33mapplication/x-www-form-urlencoded: Parsed\033[0m" << std::endl;
            this->_state = BODY_COMPLETE;
            this->file.close();
            return HTTP_RES_OK;
        }

        if (this->_mediaType == "multipart/form-data") {

            std::cout << YELLOW << "multipart/form-data Processing" << WHITE << std::endl;
            std::cout << "boundary: " << this->_boundary << std::endl;

            std::string line;

            while (this->_mpState != MP_COMPLETE) {

                if (!std::getline(this->file, line)) {
                    return HTTP_RES_BAD_REQUEST;
                }

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

                        this->file.close();
                        return HTTP_RES_OK;
                    }
                    continue;
                }

                if (this->_mpState == MP_READING_HEADERS) {
                    if (line.empty()) {

                        if (this->_partLevelHeaders.find("Content-Disposition") ==
                                this->_partLevelHeaders.end() ||
                            this->_partLevelHeaders["Content-Disposition"].size() != 1) {
                            return HTTP_RES_BAD_REQUEST;
                        }

                        std::string contentDispositionValue =
                            this->_partLevelHeaders["Content-Disposition"][0];
                        this->_currentPart.name = extractPar(contentDispositionValue, "name");
                        this->_currentPart.filename =
                            extractPar(contentDispositionValue, "filename");

                        // if filename is not provided meaning it's just a form value
                        if (this->_currentPart.filename.empty() == false) {
                            this->_currentPart.filePath =
                                this->conf->getUploadPath() + "/" + this->_currentPart.filename;
                        }

                        // if contentType found add it
                        if (this->_partLevelHeaders.find("Content-Type") !=
                                this->_partLevelHeaders.end() &&
                            this->_partLevelHeaders["Content-Type"].empty() == false) {
                            this->_currentPart.contentType =
                                this->_partLevelHeaders["Content-Type"][0];
                        }

                        /* open output file if exist */
                        if (this->_currentPart.filename.empty() == false) {
                            this->_currentPart.file.open(this->_currentPart.filePath.c_str(),
                                                         std::ios::out | std::ios::binary);
                            if (this->_currentPart.file.is_open() == false) {
                                std::cerr << RED << "Error: Unable to open file "
                                          << this->_currentPart.filePath << WHITE << std::endl;
                                return HTTP_RES_INTERNAL_SERVER_ERROR;
                            }
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
                    /* Check if got boundary, if so save the file then store it in the vector */
                    if ("--" + this->_boundary == line || "--" + this->_boundary + "--" == line) {

                        if (this->_mpCurrentLine.empty() == false) {
                            size_t writeLen = this->_mpCurrentLine.size();
                            // Remove \n and \r from the end of the buffer before
                            // writing
                            if (writeLen >= 1 && this->_mpCurrentLine[writeLen - 1] == '\n')
                                writeLen--;
                            if (writeLen >= 1 && this->_mpCurrentLine[writeLen - 1] == '\r')
                                writeLen--;

                            // if no filename write in the buffer
                            if (this->_currentPart.filename.empty() == false) {
                                this->_currentPart.file.write(this->_mpCurrentLine.c_str(),
                                                              writeLen);
                            }
                            else {
                                this->_currentPart.buffer += this->_mpCurrentLine;
                            }
                        }
                        if (this->_currentPart.filename.empty() == false) {
                            this->_currentPart.file.close();
                        }

                        this->_formFields[this->_currentPart.name].push_back(this->_currentPart);

                        /* if normal boundary found reset the state machine */
                        if ("--" + this->_boundary == line) {
                            this->_mpState = MP_READING_HEADERS;
                            this->_currentPart = FilePart();
                            this->_partLevelHeaders.clear();
                            continue;
                        }
                        /* if end boundary found set body complete */
                        this->_mpState = MP_COMPLETE;

                        this->file.close();
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
        }
        else {
            std::string fileExtension = "";
            if (httpParser->_mimeToExt.find(this->_mediaType) == httpParser->_mimeToExt.end()) {
                fileExtension = ".bin";
            }
            else {
                fileExtension = httpParser->_mimeToExt[this->_mediaType];
            }

            // add Extension to file name
            std::string newFileWithExtension = this->filePath + fileExtension;

            if (std::rename(this->filePath.c_str(), newFileWithExtension.c_str()) != 0) {
                std::cerr << "Error: renaming file" << std::endl;
                return HTTP_RES_INTERNAL_SERVER_ERROR;
            }
            this->_state = BODY_COMPLETE;
            return HTTP_RES_OK;
        }
    }
    return HTTP_RES_OK;
}

/*---- Body public member functions ---- */

void HttpParser::Body::init(HttpParser *httpParser)
{
    this->httpParser = httpParser;

    this->conf = new ResolvedConfig(httpParser->getRequestUriPath(), httpParser->_config);
    this->_maxBodySize = this->conf->getBodySize();
    _normalizePath();
}

/*---- Body private member functions ---- */

void HttpParser::Body::_normalizePath(void)
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
    for (std::vector<std::string>::iterator it = elements.begin(); it != elements.end(); it++) {
        normPath += "/";
        normPath += *it;
    }
    if (normPath == "")
        normPath = "/";
    // std::cout << "The normalized path is :'" << normPath << "'" << std::endl;
    this->normalizedPath = normPath;
}

std::string HttpParser::Body::_resolveFilePath(std::string &upload_store)
{
    std::string uri = this->normalizedPath;
    std::cout << "This is the normalized Path: " << uri << std::endl;
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

HttpParser::httpResult HttpParser::Body::_initParsingInfo()
{

    /* NOTE: When POST request rout is a script, body shouldn't be parsed */
    if (isCgi(httpParser->getRequestUriRawPath(), this->conf->getCgi()) == true) {
        // don't validate path just create a random file
        srand(time(NULL));
        std::stringstream ss;
        ss << "/tmp/" << "upload_" << time(NULL) << "_" << rand();
        this->filePath = ss.str();
    }
    else {
        std::string uploadPath = this->conf->getUploadPath();
        std::cout << RED << "Upload path : " << uploadPath << WHITE << std::endl;

        if (uploadPath.empty()) {
            return HTTP_RES_METHOD_NOT_ALLOWED;
        }

        if (access(uploadPath.c_str(), R_OK) == -1) {
            std::cerr << RED << "Error: Unable to open file " << uploadPath << WHITE << std::endl;
            return HTTP_RES_INTERNAL_SERVER_ERROR;
        }

        /* NOTE: Fixed a bug here When fileuPath is a directory */
        std::string filePath = _resolveFilePath(uploadPath);
        if (filePath.empty() == true || filePath[filePath.size() - 1] == '/') {
            std::stringstream ss;
            srand(time(NULL));
            ss << filePath << "upload_" << time(NULL) << "_" << rand();
            filePath = ss.str();
        }
        this->filePath = filePath;
    }

    file.open(this->filePath.c_str(), std::ios::out | std::ios::in | std::ios::trunc);
    if (file.is_open() == false) {
        std::cerr << "Error: " << this->filePath << " Failed to open file" << std::endl;
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

            // NOTO: Since RFC allows boundaries to be either with or without quotes
            std::string boundary = contentType.substr(contentType.find("boundary=") + 9);

            if (boundary.size() >= 2 && boundary[0] == '"' &&
                boundary[boundary.size() - 1] == '"') {
                boundary = boundary.substr(1, boundary.size() - 2);
            }
            this->_boundary = boundary;
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

        size_t contentLengthValue = std::atol(contentLength[0].c_str());

        this->_type = CONTENT_LENGTH;
        /*if (contentLengthValue < 0) {
            return HTTP_RES_LENGTH_REQUIRED;
        }*/
        if (contentLengthValue == 0) {

            this->_state = BODY_COMPLETE; /* Since Body is 0 Nothing to read */
            this->_contentLength = 0;
            return HTTP_RES_OK;
        }

        if (contentLengthValue > this->_maxBodySize) {
            return HTTP_RES_REQUEST_ENTITY_TOO_LARGE;
        }
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
    this->_chunkState = CHUNK_READ_SIZE;
    return HTTP_RES_OK;
}

std::string HttpParser::Body::extractPar(const std::string header, const std::string key)
{
    size_t pos = header.find(key);
    if (pos == std::string::npos)
        return "";

    pos += key.length();
    while (pos < header.size() && (header[pos] == ' ' || header[pos] == '='))
        pos++;

    if (pos >= header.size())
        return "";

    if (header[pos] == '"') {
        pos++;
        size_t end = header.find('"', pos);
        if (end == std::string::npos)
            return "";
        return header.substr(pos, end - pos);
    }
    else {
        size_t end = header.find(';', pos);
        if (end == std::string::npos)
            end = header.size();
        std::string value = header.substr(pos, end - pos);
        return trim(value);
    }
}

void HttpParser::Body::_fileCleanUp()
{
    file.close();
    if (this->filePath.empty() == false) {
        remove(this->filePath.c_str());
    }
}

/* ===================================================================================== */
/* ------------------- NESTED HttpParser -> Body CLASS FilePart  ----------------------- */
/* ===================================================================================== */

HttpParser::FilePart::FilePart() {}

HttpParser::FilePart::FilePart(const FilePart &other)
    : name(other.name), filename(other.filename), contentType(other.contentType),
      filePath(other.filePath), buffer(other.buffer)
{
    if (!other.filePath.empty() && other.file.is_open()) {
        // Re-open a new stream on the same path with same flags
        this->file.open(other.filePath.c_str());
    }
}

HttpParser::FilePart &HttpParser::FilePart::operator=(const FilePart &other)
{
    if (this != &other) {
        // Close existing stream first
        if (file.is_open())
            file.close();

        name = other.name;
        filename = other.filename;
        contentType = other.contentType;
        filePath = other.filePath;
        buffer = other.buffer;

        if (!other.filePath.empty() && other.file.is_open()) {
            file.open(other.filePath.c_str());
        }
    }
    return *this;
}

HttpParser::FilePart::~FilePart()
{
    if (filename.empty() == false && file.is_open())
        file.close();
}

void HttpParser::FilePart::printInfo() const
{
    std::cout << "name:         " << this->name << std::endl;
    std::cout << "filename:     " << this->filename << std::endl;
    std::cout << "contentType:  " << this->contentType << std::endl;
    std::cout << "buffer:       " << this->buffer << std::endl;
}

/* ===================================================================================== */
/* ---------------------- PRIVATE CORE PARSING LOGI HttpParser ------------------------- */
/* ===================================================================================== */

HttpParser::httpResult HttpParser::_parseRequestLine(std::string requestLine)
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

    this->_requestType = getRequestTypeFromString(requestType);
    if (this->_requestType == UNKNOWN) {
        return HTTP_RES_BAD_REQUEST;
    }

    if ((this->_requestType == GET || this->_requestType == POST || this->_requestType == DELETE) ==
        false) {
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

HttpParser::httpResult
HttpParser::_parseRequestHeaderName(std::string headerField,
                                    std::map<std::string, std::vector<std::string> > &_headers)
{

    size_t colonPos = headerField.find_first_of(":");
    if (colonPos == std::string::npos) {
        return HttpParser::HTTP_RES_BAD_REQUEST;
    }

    std::string fieldName = headerField.substr(0, colonPos);
    headerField.erase(0, colonPos + 1);
    if (_isValidToken(fieldName) == false) {
        return HttpParser::HTTP_RES_BAD_REQUEST;
    }

    std::string fieldValue = trim(headerField);

    // if the key doesn't exist
    if (_headers.find(fieldName) == _headers.end()) {
        std::vector<std::string> tmp;
        tmp.push_back(fieldValue);
        _headers.insert(std::pair<std::string, std::vector<std::string> >(fieldName, tmp));
        return HttpParser::HTTP_RES_OK;
    }
    // key already exist
    _headers[fieldName].push_back(fieldValue);
    return HttpParser::HTTP_RES_OK;
}

HttpParser::httpResult
HttpParser::_generateHeadersMap(std::string headerLines,
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

HttpParser::httpResult HttpParser::_parseRequestHeaderValue()
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

/* ===================================================================================== */
/* --------------------------------- STATIC DEBUG -------------------------------------- */
/* ===================================================================================== */

const char *HttpParser::httpResultToString(httpResult result)
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

void HttpParser::printHttpResult(httpResult result)
{
    std::cerr << "HTTP: " << httpResultToString(result) << std::endl;
}

void HttpParser::printHeaders(std::map<std::string, std::vector<std::string> > _headers)
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

void HttpParser::printRaw(const std::string str)
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

/* ===================================================================================== */
/* ------------------------------ PRIVATE UTILITY HELPERS ------------------------------  */
/* ===================================================================================== */

char HttpParser::_hexToChar(std::string hex)
{
    int ret;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> ret;
    return static_cast<char>(ret);
}

bool HttpParser::_isHex(char c)
{
    return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

bool HttpParser::_isValidPercentEncoding(std::string str)
{
    if (str.length() != 2)
        return false;
    return _isHex(str[0]) && _isHex(str[1]);
}

void HttpParser::_replacePlusWithSpace(std::string &str)
{
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '+') {
            str[i] = ' ';
        }
    }
}

std::string HttpParser::_urlDecode(std::string str)
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

void HttpParser::_parseQueryString(std::string &query, std::map<std::string, std::string> &store)
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
            _replacePlusWithSpace(key);
            _replacePlusWithSpace(value);
            key = _urlDecode(key);
            value = _urlDecode(value);
            store[key] = value;
        }
    }
}

bool HttpParser::_isValidToken(const std::string &str)
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

std::vector<std::string> HttpParser::_getTokens(std::string line, std::string delimiter)
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

HttpParser::requestType HttpParser::getRequestTypeFromString(std::string _requestType)
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
