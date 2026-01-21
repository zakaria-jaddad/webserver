#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "../server/Server.hpp"
#include "ResolvedConfig.hpp"

struct t_client;

class Response{

	ResolvedConfig	conf;
	std::string			normalizedPath;
	t_client				*client;
	bool						wait;

public:
	Response( t_client* client );

	//HttpParser::httpResult	checkMethod( void );
	void normalizePath( void );
	int	httpDelete( void );

};

#endif
