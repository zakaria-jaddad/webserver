#include "../../includes/response/Response.hpp"

std::string Response::getStatusMessage(int code)
{
    switch (code)
    {
        case 200: return "OK";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 307: return "Temporary Redirect";
        case 308: return "Permanent Redirect";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 408: return "Request Timeout";
        case 500: return "Internal Server Error";
        case 405: return "Method Not Allowed";
        case 504: return "Gateway timeout";
        case 501: return "Not Implemented";
        case 505: return "HTTP Version Not Supported";
        default:  return "Error";
    }
}

// crafting response
void Response::craftPageResponse(int code, int fd)
{

    if (static_cast<size_t>(client->writeOffset) < client->outBuffer.size())
    {
        client->writeOffset += sendResponse(client->outBuffer.data() + client->writeOffset, 
                                            client->outBuffer.size() - client->writeOffset);
        return;
    }

    MIME mime;

    if (!this->headerFlag) {
        sendHeader(code, mime.getMIME(".html"), "", true);
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
void Response::craftPageResponse(int code, std::string &location)
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
        sendHeader(code, "", location, true);
        this->headerFlag = 1;
    }

    std::stringstream ss;
    ss << std::hex << html.length();
    std::string hexStr = ss.str();
    hexStr += "\r\n";
    sendResponse(hexStr.c_str(), hexStr.size());

    std::cout << "++++++++sending body++++++++" << std::endl;
    std::cout << html << std::endl;

    sendResponse(html.c_str(), html.size());
}

// serving pages
int Response::servePages(int code)
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

    craftPageResponse(code, fd);

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
int Response::servePages(int code, std::string &location)
{

    std::cout << "---------------------Serving error" << code << " ------------" << std::endl;

    craftPageResponse(code, location);

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
void Response::sendHeader(int status_code, std::string content_type, std::string location, bool use_chunked)
{
    // std::cout << "++++++++sending header++++++++" << std::endl;

    std::stringstream ss;


    ss << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";

    if (!content_type.empty())
      ss << "Content-Type: " << content_type << "\r\n";
    if (!location.empty())
      ss << "Location: " << location << "\r\n";
    if (use_chunked)
      ss << "Transfer-Encoding: chunked\r\n";
    
    ss << "\r\n";

    // std::cout << ss.str() << std::endl;
    client->outBuffer += ss.str();
}

bool Response::isComplete(void) { return (this->status); }

bool Response::isError(void) { return (this->error); }
