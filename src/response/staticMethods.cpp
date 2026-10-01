#include "../../includes/response/Response.hpp"
#include <cerrno> //todelete
#include <dirent.h>
#include <sys/stat.h> //todelete
#include <unistd.h>   //todelete

HttpParser::httpResult Response::httpPost()
{
    servePages(200, 0);
    return HttpParser::HTTP_RES_OK;
}

HttpParser::httpResult Response::httpDelete(void)
{
    std::string absPath = this->conf.getRoot();

    std::string uri = this->client->request->getRequestUriPath();
    absPath += normalizePath(uri);
    if (access(absPath.c_str(), W_OK)) {
        return HttpParser::HTTP_RES_NOT_FOUND;
    }
    if (unlink(absPath.c_str())) {
        return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
    }

    servePages(204, 0);
    return HttpParser::HTTP_RES_OK;
}

std::string Response::chooseIndex(std::string &path)
{
    std::vector<std::string> indx = this->conf.getIndex();

    for (std::vector<std::string>::iterator it = indx.begin(); it != indx.end(); it++) {
        std::string tst = path + "/" + *it;
        if (access(tst.c_str(), R_OK) == 0)
            return ("/" + *it);
    }

    return "";
}

std::string Response::generateDirectoryListingHTML(const std::string &path,
                                                   const std::string &requestPath,
                                                   std::vector<std::string> entries)
{
    std::ostringstream html;

    html << "<!DOCTYPE html>\n<html>\n<head>\n";
    html << "<title>Index of " << requestPath << "</title>\n";
    html << "</head>\n<body>\n";
    html << "<h1>Index of " << requestPath << "</h1>\n";
    html << "<hr>\n<ul>\n";

    std::vector<std::string>::iterator dir_it = entries.begin();

    for (; dir_it != entries.end(); dir_it++) {
        std::string fullPath = path + "/" + *dir_it;
        struct stat statbuf;

        if (stat(fullPath.c_str(), &statbuf) == 0) {
            // forging the html for the folder entries
            std::string href = requestPath + "/" + *dir_it;
            href = normalizePath(href);
            html << "<li><a href=\"" << href;
            if (S_ISDIR(statbuf.st_mode)) {
                html << "/";
            }
            html << "\">" << *dir_it;
            if (S_ISDIR(statbuf.st_mode)) {
                html << "/";
            }
            html << "</a></li>\n";
        }
    }
    html << "</ul>\n<hr>\n</body>\n</html>";
    return html.str();
}

void Response::serveDirectoryContent(std::string &absPath)
{
    DIR *dir = opendir(absPath.c_str());
    std::vector<std::string> entries;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..")
            continue;
        entries.push_back(entry->d_name);
    }
    closedir(dir);

    std::string html = generateDirectoryListingHTML(absPath, this->normalizedPath, entries);

    sendHeader(200, "text/html", "", true, this->closeConnection);

    craftChunkSize(html.length());

    client->outBuffer.append(html);
    client->outBuffer.append("\r\n", 2);
    client->outBuffer.append("0\r\n\r\n", 5);

    ssize_t sent = sendResponse(client->outBuffer.data() + client->writeOffset,
                                client->outBuffer.size() - client->writeOffset);
    if (sent == -2)
        return;
    else if (sent > 0)
        client->writeOffset += sent;
}

HttpParser::httpResult Response::httpGet(void)
{
    char buff[READ_BYTES] = {0};
    ssize_t r = 0;
    int fd;
    std::string ext;

    // Drain any buffered data left from a previous partial send first.
    if (static_cast<size_t>(client->writeOffset) < client->outBuffer.size()) {
        ssize_t sent = sendResponse(client->outBuffer.data() + client->writeOffset,
                                    client->outBuffer.size() - client->writeOffset);
        if (sent == -2)
            return HttpParser::HTTP_RES_OK;
        else if (sent > 0)
            client->writeOffset += sent;
        if (static_cast<size_t>(client->writeOffset) < client->outBuffer.size())
            return HttpParser::HTTP_RES_OK;
        client->outBuffer.clear();
        client->writeOffset = 0;
    }

    if (this->fileFD == -3) {
        std::string absPath = this->conf.getRoot();
        std::string uri = this->client->request->getRequestUriPath();
        absPath += normalizePath(uri);

        struct stat info;
        std::cout << YELLOW << "Looking for your requested resource..." << WHITE << std::endl;

        if (stat(absPath.c_str(), &info) != 0) {
            return HttpParser::HTTP_RES_NOT_FOUND;
        }

        if (S_ISDIR(info.st_mode)) {
            std::string index_to_serve = chooseIndex(absPath);

            if (index_to_serve.empty()) {
                if (!conf.getDirectoyListing()) {
                    return HttpParser::HTTP_RES_FORBIDDEN;
                }
                else {
                    serveDirectoryContent(absPath);
                    if (error) {
                        return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
                    }
                    this->status = 1;
                    return HttpParser::HTTP_RES_OK;
                }
            }
            else
                absPath += index_to_serve;
        }

        ext = getExtension(absPath);
        std::cout << "extension to serve : " << ext << std::endl;

        if (access(absPath.c_str(), R_OK)) {
            return HttpParser::HTTP_RES_NOT_FOUND;
        }

        fd = open(absPath.c_str(), O_RDONLY);
        if (fd == -1) {
            return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
        }
        this->fileFD = fd;
    }

    MIME mime;
    std::string m = mime.getMIME(ext);
    if (!this->headerFlag) {
        sendHeader(200, m, "", true, this->closeConnection);
        this->headerFlag = 1;
    }

    // Send as many chunks as the socket will accept in one go. This keeps
    // pipelined responses flowing without yielding back to epoll between
    // every chunk.
    while (true) {
        r = read(this->fileFD, buff, READ_BYTES);
        if (r == -1) {
            close(this->fileFD);
            this->error = 1;
            return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
        }
        else if (r == 0) {
            client->outBuffer.append("0\r\n\r\n", 5);
            ssize_t sent = sendResponse(client->outBuffer.data() + client->writeOffset,
                                        client->outBuffer.size() - client->writeOffset);
            if (sent == -2)
                return HttpParser::HTTP_RES_OK;
            else if (sent > 0)
                client->writeOffset += sent;
            if (static_cast<size_t>(client->writeOffset) == client->outBuffer.size()) {
                std::cout << "done" << std::endl;
                close(this->fileFD);
                this->fileFD = -3;
                this->status = 1;
                client->outBuffer.clear();
                client->writeOffset = 0;
            }
            return HttpParser::HTTP_RES_OK;
        }

        craftChunkSize(r);
        client->outBuffer.append(buff, r);
        client->outBuffer.append("\r\n", 2);
        ssize_t sent = sendResponse(client->outBuffer.data() + client->writeOffset,
                                    client->outBuffer.size() - client->writeOffset);
        if (sent == -2)
            return HttpParser::HTTP_RES_OK;
        else if (sent > 0)
            client->writeOffset += sent;
        if (static_cast<size_t>(client->writeOffset) < client->outBuffer.size())
            return HttpParser::HTTP_RES_OK;
        client->outBuffer.clear();
        client->writeOffset = 0;
    }
}
