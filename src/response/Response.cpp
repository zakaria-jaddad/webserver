#include "../../includes/response/Response.hpp"

Response::Response( const t_client* dep )
:conf(dep->request->getRequestURIPath(), dep->config)
{
    
}