#include "includes/response/Response.hpp"
#include "includes/request/request.hpp"
#include <stdio.h>

Response::Response( t_client* dep )
:	conf(dep->request->getRequestURIPath(), dep->config)
{
	this->wait = 0;
	this->client = dep;
}

//HttpParser::httpResult Response::checkMethod( void )
//{
//  std::string methods[] = {"GET", "POST", "DELETE", "HEAD", "PUT", "OPTIONS" ,"TRACE", "CONNECT"};
//
//	HttpParser::requestType req_method = request.getRequestType();
//}

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

int Response::httpDelete( void ) {
	normalizePath();
	return (0);
}
