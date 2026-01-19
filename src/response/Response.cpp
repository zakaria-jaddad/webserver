#include "includes/response/Response.hpp"
#include "includes/request/request.hpp"

Response::Response( const t_client* dep )
:	conf(dep->request->getRequestURIPath(), dep->config)
{
    
}

HttpParser::httpResult Response::checkMethod( void )
{
  std::string methods[] = {"GET", "POST", "DELETE", "HEAD", "PUT", "OPTIONS" ,"TRACE", "CONNECT"};

	HttpParser::requestType req_method = request.getRequestType();

	if (this->conf.getAcceptedMethods().empty())
	{
		if ()
	}
	else
	{

	}

}
