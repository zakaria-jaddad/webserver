#include "includes/response/Response.hpp"

int Response::httpDelete( void ) {
	//TODO:check if method is allowed
	std::string absPath = this->conf.getRoot();

	normalizePath();
	absPath += this->normalizedPath;
	std::cout << "file to delete : " << absPath << std::endl;
	if (access(absPath.c_str(), W_OK)) {
		sendResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 3\r\n\r\n404");
		this->error = 1;
		return (1);
	}
	if (unlink(absPath.c_str())) {
		sendResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 3\r\n\r\n404");
		this->error = 1;
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
			sendResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 3\r\n\r\n404");
			this->error = 1;
			return (1);
		}
		fd = open(absPath.c_str(), O_RDONLY);
		if (fd == -1) {
			sendResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 3\r\n\r\n404");
			this->error = 1;
			return (1);
		}
		this->fileFD = fd;
	}
	if (!this->headerFlag) {
		sendHeader();
		this->headerFlag = 1;
		return (0);
	}
	r = read(this->fileFD, buff, 1023);
	if (r == -1) {
		//cleanup
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