#include "../../includes/response/Response.hpp"
#include <ostream>
#include <stdexcept>
#include <sys/stat.h>

// std::string Response::craftHeaderStatus(std::string &header)
//{
//     std::string line = "HTTP/1.1";
//
//     size_t status_pos = header.find("status");
//
//     if (status_pos == std::string::npos)
//         line += " 200 OK\r\n";
//     else {
//         size_t line_end = header.find("\r\n", status_pos);
//         std::string status = header.substr(status_pos + 7, line_end - (status_pos + 7));
//         line.append(status);
//         header.erase(status_pos, line.size());
//     }
//
//     return line;
// }

std::string Response::craftHeaderStatus(std::string &header)
{
    const std::string defaultStatus = "HTTP/1.1 200 OK\r\n";

    // CGI header names are case-insensitive, but most scripts emit "Status:"
    size_t statusPos = header.find("Status:");
    if (statusPos == std::string::npos)
        return defaultStatus;

    size_t valuePos = statusPos + strlen("Status:");

    // Skip optional whitespace after the colon
    while (valuePos < header.size() && (header[valuePos] == ' ' || header[valuePos] == '\t'))
        ++valuePos;

    size_t lineEnd = header.find("\r\n", valuePos);
    if (lineEnd == std::string::npos)
        lineEnd = header.size();

    std::string status = header.substr(valuePos, lineEnd - valuePos);

    // Remove the entire Status header, including the trailing CRLF if present
    if (lineEnd + 2 <= header.size())
        header.erase(statusPos, lineEnd + 2 - statusPos);
    else
        header.erase(statusPos);

    return "HTTP/1.1 " + status + "\r\n";
}

// void Response::craftHeader(std::string &buff)
//{
//     std::string headers = buff;
//     size_t clrf_pos = buff.find(DOUBLE_CRLF);
//
//     if (clrf_pos != std::string::npos) {
//         headers = buff.substr(0, clrf_pos);
//         buff.erase(0, headers.size() + strlen(DOUBLE_CRLF));
//
//         client->outBuffer.append(headers);
//
//         std::cout << "+++++++ header to send next ++++++++" << std::endl;
//         std::cout << head_resp << std::endl;
//
//         client->outBuffer.append(CRLF);
//         this->_phase = BODY;
//     }
//     else {
//         client->outBuffer.append(headers);
//
//         std::cout << "+++++++ crlf not found header to send next ++++++++" << std::endl;
//         std::cout << head_resp << std::endl;
//
//         buff.erase();
//     }
//
//     if (this->_phase == BODY) {
//         std::string line = craftHeaderStatus(headers);
//         client->outBuffer.insert(0, line);
//         client->outBuffer.append("Transfer-Encoding: chunked\r\n");
//         client->outBuffer.append(DOUBLE_CRLF);
//
//         std::cout << "+++++++ header to send next ++++++++" << std::endl;
//         std::cout << client->outBuffer << std::endl;
//     }
//
//     std::cout << "crafted header" << std::endl;
// }

void Response::craftHeader(std::string &buff)
{
    cgiHeaderBuffer += buff;

    size_t headerEnd = cgiHeaderBuffer.find(DOUBLE_CRLF);
    if (headerEnd == std::string::npos)
        return;

    std::string headers = cgiHeaderBuffer.substr(0, headerEnd);
    if (!headers.empty() && headers.compare(headers.size() - 2, 2, "\r\n") != 0) {
        headers += "\r\n";
    }
    cgiHeaderBuffer.erase(0, headerEnd + strlen(DOUBLE_CRLF));

    std::string statusLine = craftHeaderStatus(headers);

    client->outBuffer.clear();
    client->outBuffer += statusLine;
    client->outBuffer += headers;
    client->outBuffer += "Transfer-Encoding: chunked\r\n";
    if (this->closeConnection)
        client->outBuffer += "Connection: close\r\n";
    client->outBuffer += "\r\n";

    // std::cout << GREEN << "===========crafted header===========" << WHITE << std::endl;
    // std::cout << client->outBuffer << std::endl;

    _phase = BODY;
}

void Response::craftBody(std::string &data, size_t len)
{
    std::stringstream ss;
    ss << std::hex << len;

    client->outBuffer += ss.str();
    client->outBuffer += "\r\n";
    client->outBuffer.append(data.c_str(), len);
    client->outBuffer += "\r\n";

    // std::cout << GREEN << "===========crafted body===========" << WHITE << std::endl;
    // std::cout << client->outBuffer << std::endl;
}

// void Response::craftBody(std::string &buff)
//{
//     resp.append(buff);
//     resp.append(CRLF);
//     buff.erase();
//
//     std::cout << "crafted body : " << std::endl;
//     std::cout << resp << std::endl;
// }

HttpParser::httpResult Response::craftResponse()
{
    char buff[READ_BYTES];

    ssize_t bytes = read(cgi_out_pipe[0], buff, sizeof(buff));

    if (bytes == -1) {
        perror("read");
        error = true;
        return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
    }

    if (bytes == 0) {
        client->outBuffer.append("0\r\n\r\n");
        _phase = COMPLETE;
        return HttpParser::HTTP_RES_OK;
    }

    if (_phase == HEADERS) {
        std::string hd(buff, bytes);
        craftHeader(hd);

        // Header may still be incomplete
        if (_phase == HEADERS)
            return HttpParser::HTTP_RES_OK;

        // Header just completed.
        // cgiHeaderBuffer now contains any body bytes that followed it.
        if (!cgiHeaderBuffer.empty()) {
            // std::cout << "Remaining after header:\n";
            // std::cout.write(cgiHeaderBuffer.data(), cgiHeaderBuffer.size());
            // std::cout << "\nsize = " << cgiHeaderBuffer.size() << std::endl;
            chunk_size = cgiHeaderBuffer.size();
            craftBody(cgiHeaderBuffer, chunk_size);
            cgiHeaderBuffer.clear();
        }

        return HttpParser::HTTP_RES_OK;
    }

    // BODY
    std::string body(buff, bytes);
    craftBody(body, bytes);

    return HttpParser::HTTP_RES_OK;
}

// HttpParser::httpResult Response::craftResponse(void)
//{
//     std::cout << "--------------start of crafting response----------------" << std::endl;
//
//     char buff[READ_BYTES];
//     std::string val;
//
//     while (this->_phase == HEADERS && !this->error) {
//         ssize_t bytes = read(cgi_out_pipe[0], buff, sizeof(buff) - 1);
//         std::cout << GREEN << "READING..." << WHITE << std::endl;
//
//         if (bytes == 0) {
//             std::cout << "nothing else to read" << std::endl;
//             this->_phase = COMPLETE;
//             break;
//         }
//
//         if (bytes == -1) {
//             perror("read failed");
//             error = true;
//             return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
//         }
//
//         buff[bytes] = 0;
//
//         val = buff;
//
//         craftHeader(val);
//     }
//
//     if (this->_phase == BODY) {
//         ssize_t bytes;
//         if (val.empty()) {
//             bytes = read(cgi_out_pipe[0], buff, READ_BYTES);
//
//             if (bytes == 0) {
//                 this->_phase = COMPLETE;
//                 return HttpParser::HTTP_RES_OK;
//             }
//
//             if (bytes == -1) {
//                 perror("read failed");
//                 error = true;
//                 return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
//             }
//
//             // buff[bytes] = 0;
//
//             val.assign(buff, bytes);
//         }
//
//         chunk_size = bytes;
//
//         craftBody(val);
//     }
//
//     std::cout << "--------------end of crafting response----------------" << std::endl;
//
//     return HttpParser::HTTP_RES_OK;
// }

void printRaw(const std::string str)
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

ssize_t Response::sendResponse(const char *resp, size_t size)
{
    if (size == 0)
        return 0;

    ssize_t bytes_sent = send(client->fd, resp, size, 0);

    if (bytes_sent == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return -2;

        std::cerr << "failed to send" << std::endl;
        this->error = 1;
        return -1;
    }

    return bytes_sent;
}
