#include "includes/response/Response.hpp"

std::string Response::getStatusMessage(int code)
{
    switch (code)
    {
        case 200: return "OK";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        case 405: return "Method Not Allowed";
        case 501: return "Not Implemented";
        case 505: return "HTTP Version Not Supported";
        default:  return "Error";
    }
}

void Response::craftErrorResponse(int code, int fd)
{
    if (!this->headerFlag)
    {
        sendHeader(code);
        this->headerFlag = 1;
    }
   
    char buff[1024];
    int bytes = read(fd, buff, sizeof(buff)-1);
    if (bytes == 0)
    {
        std::cout << "Done reading" << std::endl ;
        this->error = 1;
        this->_phase = ERROR;
        return ;
    }

    if (bytes == -1)
    {
        // fallback if no custom file
        sendResponse("HTTP/1.1 500 Internal Server Error\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: 21\r\n\r\n"
                     "Internal Server Error");
        return;
    }

    chunk_size = bytes;
    buff[bytes] = 0;

    std::stringstream ss ;
    ss << std::hex << chunk_size;
    std::string hexStr = ss.str();
    hexStr += "\r\n";
    sendResponse(hexStr.c_str());
    
    std::cout << "++++++++sending body++++++++" << std::endl;
    std::cout << buff << std::endl;
    
    sendResponse(buff);
}

int Response::serveError(int code)
{
    static int fd = -1;
    std::string errorPath = this->conf.getErrorPages(code);

    std::cout << "error to serve : " << errorPath << std::endl;

    if (errorPath.empty())
    {
        // fallback if no custom file
        sendResponse("HTTP/1.1 500 Internal Server Error\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: 21\r\n\r\n"
                     "Internal Server Error");
        return 1;
    }

    if (fd == -1)
        fd = open(errorPath.c_str(), O_RDONLY);

    if (fd == -1)
    {
        // fallback if no custom file
        sendResponse("HTTP/1.1 500 Internal Server Error\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: 21\r\n\r\n"
                     "Internal Server Error");
        return 1;
    }

    craftErrorResponse(code, fd);

    if (this->_phase == ERROR)
    {
        sendResponse("0\r\n\r\n");
        close(fd);
        fd = -1;
    }


    return 0;
}


void Response::normalizePath(void)
{
    std::stringstream ss(this->client->request->getRequestUriPath());
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
    std::cout << "The normalized path is :'" << normPath << "'" << std::endl;
    this->normalizedPath = normPath;
}

void Response::sendHeader(int status_code)
{
    std::cout << "++++++++sending header++++++++" << std::endl;

    std::stringstream ss;

    ss << "HTTP/1.1 " << status_code << " "
       << getStatusMessage(status_code) << "\r\n";

    ss << "Content-Type: text/html\r\n";
    ss << "Transfer-Encoding: chunked\r\n";
    ss << "\r\n";

    std::cout << ss.str() << std::endl;
    sendResponse(ss.str().c_str());
}

bool Response::isComplete(void) { return (this->status); }

bool Response::isError(void) { return (this->error); }
