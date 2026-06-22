#include "../../includes/response/Response.hpp"

std::string Response::getStatusMessage(int code)
{
  switch (code)
  {
    case HttpParser::HTTP_RES_CONTINUE:
      return "100 Continue";
    case HttpParser::HTTP_RES_SWITCHING_PROTOCOLS:
      return "101 Switching Protocols";

      // 2xx
    case HttpParser::HTTP_RES_OK:
      return "200 OK";
    case HttpParser::HTTP_RES_CREATED:
      return "201 Created";
    case HttpParser::HTTP_RES_ACCEPTED:
      return "202 Accepted";
    case HttpParser::HTTP_RES_NO_CONTENT:
      return "204 No Content";
    case HttpParser::HTTP_RES_PARTIAL_CONTENT:
      return "206 Partial Content";

      // 3xx
    case HttpParser::HTTP_RES_MOVED_PERMANENTLY:
      return "301 Moved Permanently";
    case HttpParser::HTTP_RES_FOUND:
      return "302 Found";
    case HttpParser::HTTP_RES_SEE_OTHER:
      return "303 See Other";
    case HttpParser::HTTP_RES_NOT_MODIFIED:
      return "304 Not Modified";
    case HttpParser::HTTP_RES_TEMPORARY_REDIRECT:
      return "307 Temporary Redirect";
    case HttpParser::HTTP_RES_PERMANENT_REDIRECT:
      return "308 Permanent Redirect";

      // 4xx
    case HttpParser::HTTP_RES_BAD_REQUEST:
      return "400 Bad Request";
    case HttpParser::HTTP_RES_UNAUTHORIZED:
      return "401 Unauthorized";
    case HttpParser::HTTP_RES_FORBIDDEN:
      return "403 Forbidden";
    case HttpParser::HTTP_RES_NOT_FOUND:
      return "404 Not Found";
    case HttpParser::HTTP_RES_METHOD_NOT_ALLOWED:
      return "405 Method Not Allowed";
    case HttpParser::HTTP_RES_NOT_ACCEPTABLE:
      return "406 Not Acceptable";
    case HttpParser::HTTP_RES_REQUEST_TIMEOUT:
      return "408 Request Timeout";
    case HttpParser::HTTP_RES_CONFLICT:
      return "409 Conflict";
    case HttpParser::HTTP_RES_GONE:
      return "410 Gone";
    case HttpParser::HTTP_RES_LENGTH_REQUIRED:
      return "411 Length Required";
    case HttpParser::HTTP_RES_PRECONDITION_FAILED:
      return "412 Precondition Failed";
    case HttpParser::HTTP_RES_REQUEST_ENTITY_TOO_LARGE:
      return "413 Payload Too Large";
    case HttpParser::HTTP_RES_REQUEST_URI_TOO_LONG:
      return "414 URI Too Long";
    case HttpParser::HTTP_RES_UNSUPPORTED_MEDIA_TYPE:
      return "415 Unsupported Media Type";
    case HttpParser::HTTP_RES_RANGE_NOT_SATISFIABLE:
      return "416 Range Not Satisfiable";
    case HttpParser::HTTP_RES_EXPECTATION_FAILED:
      return "417 Expectation Failed";
    case HttpParser::HTTP_RES_TOO_MANY_REQUESTS:
      return "429 Too Many Requests";
    case HttpParser::HTTP_RES_REQUEST_HEADER_FIELDS_TOO_LARGE:
      return "431 Request Header Fields Too Large";

      // 5xx
    case HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR:
      return "500 Internal Server Error";
    case HttpParser::HTTP_RES_NOT_IMPLEMENTED:
      return "501 Not Implemented";
    case HttpParser::HTTP_RES_BAD_GATEWAY:
      return "502 Bad Gateway";
    case HttpParser::HTTP_RES_SERVICE_UNAVAILABLE:
      return "503 Service Unavailable";
    case HttpParser::HTTP_RES_GATEWAY_TIMEOUT:
      return "504 Gateway Timeout";
    case HttpParser::HTTP_RES_HTTP_VERSION_NOT_SUPPORTED:
      return "505 HTTP Version Not Supported";
    default:
      return "Unknown version";
  }
}

// crafting response
void Response::craftPageResponse(int code, int fd, bool to_close)
{

    if (static_cast<size_t>(client->writeOffset) < client->outBuffer.size())
    {
        client->writeOffset += sendResponse(client->outBuffer.data() + client->writeOffset, 
                                            client->outBuffer.size() - client->writeOffset);
        return;
    }

    MIME mime;

    if (!this->headerFlag) {
        sendHeader(code, mime.getMIME(".html"), "", true, to_close);
        this->headerFlag = 1;
    }

    char buff[READ_BYTES];
    ssize_t bytes = read(fd, buff, sizeof(buff)-1);
    if (bytes == 0)
    {
        client->outBuffer += "0\r\n\r\n";
        client->writeOffset += sendResponse(client->outBuffer.data() + client->writeOffset,
                                            client->outBuffer.size() - client->writeOffset);
        // std::cout << "Done reading" << std::endl ;
        if (static_cast<size_t>(client->writeOffset) == client->outBuffer.size())
        {
            this->_phase = COMPLETE;
            this->headerFlag = 0;
            client->writeOffset = 0;
            client->outBuffer.clear();
        }
        return;
    }
    

    if (bytes == -1) {
        this->error = 1;
        sendResponse("HTTP/1.1 500 Internal Server Error\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: 21\r\n\r\n"
                     "Internal Server Error", 105);
        return;
    }

    chunk_size = bytes;
    // buff[bytes] = 0;

    std::stringstream ss;
    ss << std::hex << chunk_size;
    std::string hexStr = ss.str();
    hexStr += "\r\n";
    client->outBuffer += hexStr;

    // std::cout << "++++++++sending body++++++++" << std::endl;
    // std::cout << buff << std::endl;

    client->outBuffer.append(buff, bytes);
    client->outBuffer += "\r\n";
    client->writeOffset += sendResponse(client->outBuffer.data() + client->writeOffset, 
                                        client->outBuffer.size() - client->writeOffset);

    std::cout << "outbuffer : " << client->outBuffer << std::endl;
    std::cout << "writeOffset : " << client->writeOffset << " outbuffer size : " << client->outBuffer.size() << std::endl;
    // if (bytes < READ_BYTES - 1)
    // {
    //     this->_phase = COMPLETE;
    //     return;
    // }
}

// redir overload
void Response::craftPageResponse(int code, std::string &location, bool to_close)
{
  std::string html = "<!DOCTYPE html>\n"
                      "<html>\n"
                      "<head><title>Moved</title></head>\n"
                      "<body>\n"
                      "<h1>Page moved</h1>\n"
                      "<p><a href=\"" +
                      location +
                      "\">Click here to go to the new page</a></p>\n"
                      "</body>\n"
                      "</html>\n";

  if (!this->headerFlag) {
      sendHeader(code, "", location, true, to_close);
      this->headerFlag = 1;
  }

  std::stringstream ss;
  ss << std::hex << html.length();
  std::string hexStr = ss.str();
  hexStr += "\r\n";
  client->outBuffer.append(hexStr);

  // std::cout << "++++++++sending body++++++++" << std::endl;
  // std::cout << html << std::endl;

  client->outBuffer.append(html);
  client->writeOffset += sendResponse(client->outBuffer.data() + client->writeOffset, 
                                      client->outBuffer.size() - client->writeOffset);
}

// serving pages
int Response::servePages(int code, bool to_close)
{

    std::cout << "---------------------Serving page " << code << " ------------" << std::endl;

    static int fd = -1;
    std::string errorPath = this->conf.getErrorPages(code);

    if (errorPath.empty()) {
        this->error = 1;
        sendResponse("HTTP/1.1 500 Internal Server Error\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: 21\r\n\r\n"
                     "Internal Server Error", 105);
        return 1;
    }

    if (fd == -1) {
        fd = open(errorPath.c_str(), O_RDONLY);

        if (fd == -1) {
            this->error = 1;
            sendResponse("HTTP/1.1 500 Internal Server Error\r\n"
                         "Content-Type: text/plain\r\n"
                         "Content-Length: 21\r\n\r\n"
                         "Internal Server Error", 105);
            return 1;
        }
    }

    craftPageResponse(code, fd, to_close);

    if (this->error)
        return 1;

    if (this->_phase == COMPLETE) {
       
        std::cout << "outbuffer : " << client->outBuffer << std::endl;
        std::cout << "writeOffset : " << client->writeOffset << std::endl;
        close(fd);
        this->status = 1;
        fd = -1;
        std::cout << "---------------------Finished Serving error" << code << " ------------" << std::endl;
    }

    return 0;
}

// redir overload
int Response::servePages(int code, std::string &location, bool to_close)
{

    std::cout << "---------------------Serving error" << code << " ------------" << std::endl;

    craftPageResponse(code, location, to_close);

    if (this->error)
        return 1;

    this->status = 1;
    std::cout << "---------------------Finished Serving error" << code << " ------------"
              << std::endl;

    return 0;
}

// normalizing uri
void Response::normalizePath(void)
{
    std::string originalPath = this->client->request->getRequestUriPath();
    std::stringstream ss(originalPath);
    std::string elem;
    std::vector<std::string> elements;
    std::string normPath = "";
    bool ends_with_slash = false;

    if (!originalPath.empty() && originalPath[originalPath.length() - 1] == '/')
        ends_with_slash = true;

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
    
    if (ends_with_slash && normPath != "/")
        normPath += "/";

    // std::cout << "The normalized path is :'" << normPath << "'" << std::endl;
    // std::cout << "The original path is :'" << this->client->request->getRequestUriPath() << "'" << std::endl;
    this->normalizedPath = normPath;
}

// send header
// redir overload


void Response::craftChunkSize( size_t size )
{
    std::string hexStr;
    std::stringstream ss;

    ss << std::hex << size;
    hexStr = ss.str();
    hexStr += "\r\n";
    client->outBuffer.append(hexStr);
}

//original
void Response::sendHeader(int status_code, std::string content_type, std::string location, bool use_chunked, bool to_close)
{
    // std::cout << "++++++++sending header++++++++" << std::endl;

    std::stringstream ss;


    ss << "HTTP/1.1 " << getStatusMessage(status_code) << "\r\n";

    if (!content_type.empty())
      ss << "Content-Type: " << content_type << "\r\n";
    if (!location.empty())
      ss << "Location: " << location << "\r\n";
    if (use_chunked)
      ss << "Transfer-Encoding: chunked\r\n";
    if (to_close)
        ss << "Connection: close\r\n";
    
    ss << "\r\n";

    // std::cout << ss.str() << std::endl;
    client->outBuffer += ss.str();
}

bool Response::isComplete(void) { return (this->status); }

bool Response::isError(void) { return (this->error); }
