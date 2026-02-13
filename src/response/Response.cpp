#include "includes/response/Response.hpp"

Response::Response(t_client *dep)
    : conf(dep->request->getRequestUriPath(), dep->config), _phase(HEADERS)
{
    std::cout << "############### creating new response #################" << std::endl;
    
    this->wait = 0;
    this->client = dep;
    this->fileFD = -3;
    this->headerFlag = 0;
    this->status = 0;
    this->exec_status = 0;
    this->error = 0;

    if (pipe(cgi_in_pipe) == -1)
        error = true;
    
    if (pipe(cgi_out_pipe) == -1)
        error = true;
}

HttpParser::httpResult Response::checkMethod(void)
{
	if (client->request->getRequestUriPath().find(".php") != std::string::npos && !conf.getCgi().first.empty() && !conf.getCgi().second.empty())
    {
        if (!this->exec_status)
		    cgiExecution();
        
        if (error)
            return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
        
        craftResponse();
        
        if (error)
            return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;    
        
        sendingCgiResponse();

        if (error)
            return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
        
        std::cout << "Done sending the response" << std::endl;
        resp.clear();
    }
	else
	{
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
	}
	
	return (HttpParser::HTTP_RES_OK);
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

void Response::sendHeader(void)
{
    sendResponse("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
}

bool Response::isComplete(void) { return (this->status); }

bool Response::isError(void) { return (this->error); }
