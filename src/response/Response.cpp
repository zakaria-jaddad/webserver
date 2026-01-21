#include "includes/response/Response.hpp"

Response::Response( t_client* dep )
:	conf(dep->request->getRequestURIPath(), dep->config)
{
	this->wait = 0;
	this->client = dep;
	this->fileFD = -3;
	this->headerFlag = 0;
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
			sendResponse("HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/plain\r\nContent-Length: 5\r\n\r\nnn hh");
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
		sendResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 7\r\n\r\nloll hh");
		return (1);
	}
	if (unlink(absPath.c_str())) {
		sendResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 7\r\n\r\nloll hh");
		return (1);
	}
	sendResponse("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 10\r\n\r\nDeleted hh");
	return (0);
}

void Response::sendHeader( void ) {
	sendResponse("HTTP/1.1 200 OK\r\n");
}

int Response::httpGet( void ) {
	std::string absPath = this->conf.getRoot();
	normalizePath();
	absPath += this->normalizedPath;
	if (access(absPath.c_str(), W_OK)) {
		sendResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 7\r\n\r\nloll hh");
		return (1);
	}
	if (!this->headerFlag) {
		sendHeader();
		this->headerFlag = 1;
	}
	sendResponse("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 7\r\n\r\nBruh hh");
	return (0);
}
