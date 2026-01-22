#include "includes/response/Response.hpp"

Response::Response( t_client* dep )
:	conf(dep->request->getRequestURIPath(), dep->config),
	_phase(REQUEST_LINE)
{
	this->wait = 0;
	this->client = dep;
	this->fileFD = -3;
	this->headerFlag = 0;
	this->status = 0;
	this->error = 0;
}

HttpParser::httpResult Response::checkMethod( void )
{
  std::string methods[] = {"GET", "POST", "DELETE", "HEAD", "PUT", "OPTIONS" ,"TRACE", "CONNECT"};

	HttpParser::requestType req_method = client->request->getRequestType();
	switch (req_method) {
		case HttpParser::DELETE:
			httpDelete();
			break;
		case HttpParser::GET:
			httpGet();
			break;
		default:
			std::cout << "error" << std::endl;
			sendResponse("HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/plain\r\nContent-Length: 3\r\n\r\n405");
	}
	return (HttpParser::HTTP_RES_OK);
}

void Response::normalizePath( void ) {
	std::stringstream ss(this->client->request->getRequestURIPath());
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

int Response::sendResponse(const char *resp) {
	if (send(client->fd, resp, strlen(resp), 0) == -1) {
		std::cerr << "failed to send" << std::endl;
		this->error = 1;
		return (1);
	}
	return (0);
}

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

void Response::sendHeader( void ) {
	sendResponse("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
}

bool Response::isComplete( void ) {
	return (this->status);
}

bool Response::isError( void ) {
	return (this->error);
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
