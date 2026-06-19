#include "../../includes/response/Response.hpp"
#include <cerrno> //todelete
#include <dirent.h>
#include <sys/stat.h> //todelete
#include <unistd.h>   //todelete

std::string Response::resolveFilePath(std::string &upload_store)
{
    std::string uri = this->normalizedPath;
    std::string relative_path = uri;
    std::string location_prefix = conf.getLocPath();

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

// int Response::createUploadFolder( std::string& upload_path, std::string& file_path )
// {
//   size_t pos = upload_path.size();

//   while ((pos = file_path.find('/', pos)) != std::string::npos)
//   {
//     std::string dir = file_path.substr(0, pos);
//     struct stat st;
//     if (stat(dir.c_str(), &st) != 0)
//     {
//       if (mkdir(dir.c_str(), 0755) != 0 && errno != EEXIST)
//       {
//         std::cerr << "Failed to create directory: " << dir << std::endl;
//         servePages(500);
//         return 1;
//       }
//     }
//     else if (!S_ISDIR(st.st_mode))
//     {
//       std::cerr << "Path exists and is not a directory: " << dir << std::endl;
//       servePages(500);
//       return 1;
//     }
//     pos += 1;
//   }
//   return 0;
// }

HttpParser::httpResult Response::httpPost()
{
    // normalizePath();
    // std::string upload_path = conf.getUploadPath();
    // std::cout << "This is the uploaded path" << upload_path << std::endl;
    //
    // if (upload_path.empty()) {
    //     servePages(405);
    //     return 1;
    // }
    //
    // if (access(upload_path.c_str(), R_OK) == -1) {
    //     servePages(500);
    //     return 1;
    // }
    //
    // std::string file_path = resolveFilePath(upload_path);
    // std::cout << "file path : " << file_path << std::endl;
    // std::cout << "upload path : " << upload_path << std::endl;
    //
    // // if (createUploadFolder(upload_path, file_path))
    // //   return 1;
    //
    // int fd = open(file_path.c_str(), O_WRONLY | O_CREAT, 0755);
    // if (fd == -1) {
    //     perror("open failed");
    //     servePages(500);
    //     return 1;
    // }
    //
    // // placeholder for body provided by kiraje
    // std::string fake_body = "file_content=Hello+world+this+is+a+test+file";
    // write(fd, fake_body.c_str(), fake_body.length());
    // std::cout << "created!!" << std::endl;
    //
    servePages(200);
    // Edited by Zakaria

    return HttpParser::HTTP_RES_OK;
}

HttpParser::httpResult Response::httpDelete(void)
{
    std::string absPath = this->conf.getRoot();

    normalizePath();
    absPath += this->normalizedPath;
    std::cout << "file to delete : " << absPath << std::endl;
    if (access(absPath.c_str(), W_OK)) {
        return HttpParser::HTTP_RES_NOT_FOUND;
    }
    if (unlink(absPath.c_str())) {
        return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
    }

    servePages(200); // check later to change it to 204
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

std::string generateDirectoryListingHTML(const std::string &path, const std::string &requestPath,
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
            html << "<li><a href=\"" << *dir_it;
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

    sendHeader(200);

    if (error)
        return;

    craftChunkSize(html.length());

    if (error)
        return;

    sendResponse(html.c_str(), html.size());

    if (error)
        return;

    sendResponse("\r\n", 2);

    if (error)
        return;

    sendResponse("0\r\n\r\n", 5);

    if (error)
        return;
}

HttpParser::httpResult Response::httpGet(void)
{
    char buff[READ_BYTES] = {0};
    ssize_t r = 0;
    int fd;


    // std::cout << "write offset : " << client->writeOffset << std::endl;
    // std::cout << "outbuffer : " << client->outBuffer << std::endl;

    if (static_cast<size_t>(client->writeOffset) < client->outBuffer.size())
    {
        client->writeOffset += sendResponse(client->outBuffer.data() + client->writeOffset, 
                                            client->outBuffer.size() - client->writeOffset);
        return HttpParser::HTTP_RES_OK;
    }

    if (this->fileFD == -3) {
        std::string absPath = this->conf.getRoot();
        normalizePath();
        absPath += this->normalizedPath;

        std::cout << "path requested : " << absPath << std::endl;

        struct stat info;

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

        if (access(absPath.c_str(), R_OK)) {
            return HttpParser::HTTP_RES_NOT_FOUND;
        }

        fd = open(absPath.c_str(), O_RDONLY);
        if (fd == -1) {
            return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
        }
        this->fileFD = fd;
    }

    if (!this->headerFlag) {
        sendHeader(200);
        this->headerFlag = 1;
        // return HttpParser::HTTP_RES_OK;
    }

    r = read(this->fileFD, buff, READ_BYTES);
    if (r == -1) {
        close(this->fileFD);
        this->error = 1;
        return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
    }
    else if (r == 0) {
        client->outBuffer.append("0\r\n\r\n", 5);
        client->writeOffset += sendResponse(client->outBuffer.data() + client->writeOffset,
                                            client->outBuffer.size() - client->writeOffset);
        if (static_cast<size_t>(client->writeOffset) == client->outBuffer.size())
        {
            std::cout << "done" << std::endl;
            close(this->fileFD);
            this->fileFD = -3;
            this->status = 1;
            client->outBuffer.clear();
            client->writeOffset = 0;
        }
        return HttpParser::HTTP_RES_OK; // check later this looks phishy to me somehow
    }

    // buff[r] = 0;
    craftChunkSize(r);
    client->outBuffer.append(buff, r);
    client->outBuffer.append("\r\n", 2);
    client->writeOffset += sendResponse(client->outBuffer.data() + client->writeOffset, 
                client->outBuffer.size() - client->writeOffset);
    return HttpParser::HTTP_RES_OK;
}
