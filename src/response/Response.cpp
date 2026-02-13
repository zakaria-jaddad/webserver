#include "includes/response/Response.hpp"

Response::Response(t_client *dep)
    : conf(dep->request->getRequestUriPath(), dep->config), _phase(HEADERS)
{
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
