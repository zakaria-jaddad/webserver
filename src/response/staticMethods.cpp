#include "../../includes/response/Response.hpp"
#include <unistd.h>//todelete
#include <sys/stat.h>//todelete
#include <cerrno>//todelete


std::string Response::resolveFilePath( std::string& upload_store )
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


int Response::httpPost()
{
  normalizePath();
  std::string upload_path = conf.getUploadPath();

  if (upload_path.empty())
  {
    servePages(405);
    return 1;
  }

  if (access(upload_path.c_str(), R_OK) == -1)
  {
    servePages(500);
    return 1;
  }

  std::string file_path = resolveFilePath(upload_path);
	std::cout << "file path : " << file_path << std::endl;
	std::cout << "upload path : " << upload_path << std::endl;

  // if (createUploadFolder(upload_path, file_path))
  //   return 1;

  int fd = open(file_path.c_str(), O_WRONLY | O_CREAT, 0755);
  if (fd == -1)
  {
		perror("open failed");
    servePages(500);
    return 1;
  }

  //placeholder for body provided by kiraje
  std::string fake_body = "file_content=Hello+world+this+is+a+test+file";
  write(fd, fake_body.c_str(), fake_body.length());
  std::cout << "created!!" << std::endl;

  servePages(200);

  return 0;
}

int Response::httpDelete( void ) {
	//TODO:check if method is allowed
	std::string absPath = this->conf.getRoot();

	normalizePath();
	absPath += this->normalizedPath;
	std::cout << "file to delete : " << absPath << std::endl;
	if (access(absPath.c_str(), W_OK)) {
		servePages(404);
		return (1);
	}
	if (unlink(absPath.c_str())) {
		servePages(404);
		return (1);
	}
	sendResponse("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 10\r\n\r\nDeleted hh");
	return (0);
}

int Response::httpGet( void ) {
	ssize_t r = 0;
	int fd;
	char buff[1024] = {0};
	std::stringstream ss;
	std::string hexStr;
	std::string txt;

	if (this->fileFD == -3) {
		std::string absPath = this->conf.getRoot();
		normalizePath();
		absPath += this->normalizedPath;
		if (access(absPath.c_str(), W_OK)) {
			servePages(404);
			return (1);
		}
		fd = open(absPath.c_str(), O_RDONLY);
		if (fd == -1) {
			servePages(404);
			return (1);
		}
		this->fileFD = fd;
	}
	if (!this->headerFlag) {
		sendHeader(200);
		this->headerFlag = 1;
		return (0);
	}
	r = read(this->fileFD, buff, 1023);
	if (r == -1) {
		close(this->fileFD);
		this->error = 1;
		return (1);
	}
	else if (r == 0) {
		sendResponse("0\r\n\r\n");
		std::cout << "done" << std::endl;
		close(this->fileFD);
		this->fileFD = -3;
		this->status = 1;
		//cleanup
		return (2);
	}
	buff[r] = 0;
	ss << std::hex << r;
	hexStr = ss.str();
	hexStr += "\r\n";
	sendResponse(hexStr.c_str());
	sendResponse(buff);
	txt = "\r\n";
	sendResponse(txt.c_str());
	return (0);
}
