#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "ResolvedConfig.hpp"
#include "../server/Server.hpp"

class Response{

	ResolvedConfig	conf;
	HttpParser			request;

public:
	Response( const t_client* );

	HttpParser::httpResult	checkMethod( void );

};

#endif