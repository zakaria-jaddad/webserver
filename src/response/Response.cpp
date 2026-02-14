#include "../../includes/response/Response.hpp"

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

HttpParser::httpResult Response::routeMethod( HttpParser::requestType req_method)
{
	const std::string& uri = client->request->getRequestUriPath();

	bool is_php_request = false;

	if (uri.length() >= 4 && uri.compare(uri.length() - 4, 4, ".php") == 0)
			is_php_request = true;

	bool cgi_configured = !conf.getCgi().first.empty() &&
												!conf.getCgi().second.empty();

	if (is_php_request && cgi_configured)
	{
		if (!this->exec_status)
		{
			if (cgiExecution() == HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR)
			{
				servePages(500);
				return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
			}
		}

		if (craftResponse() == HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR)
		{
			servePages(500);
			return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
		}
		
		sendingCgiResponse();

		if (error)
		{
			servePages(500);
			return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
		}
		
		std::cout << "Done sending the response" << std::endl;
		resp.clear();
	}

	else
	{
		switch (req_method) {
			case HttpParser::DELETE:
				httpDelete();
				break;
			case HttpParser::GET:
				httpGet();
				break;
      case HttpParser::POST:
        httpPost();
        break;
			default:
				servePages(405);
		}
	}

	return HttpParser::HTTP_RES_OK;
}


HttpParser::httpResult Response::checkMethod(void)
{	
	HttpParser::requestType req_method = client->request->getRequestType();
	std::vector<std::string> allowed_methods = conf.getAcceptedMethods();

	if (!allowed_methods.empty() && std::find(allowed_methods.begin(), \
																						allowed_methods.end(), \
																						methodToString(req_method)) == allowed_methods.end())
	{
		servePages(405);
		return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
	}

	if (routeMethod(req_method) != HttpParser::HTTP_RES_OK)
		return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
	
	return (HttpParser::HTTP_RES_OK);
}
