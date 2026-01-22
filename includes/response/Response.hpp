#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "../server/Server.hpp"
#include "../request/request.hpp"
#include "ResolvedConfig.hpp"
#include <stdio.h>

struct t_client;

class Response{

	ResolvedConfig	conf;
	t_client			*client;
	bool				wait;
	std::string			normalizedPath;
	int				fileFD;
	int				headerFlag;
	bool				status;
	bool				error;

public:
	Response( t_client* client );

	HttpParser::httpResult	checkMethod( void );
	void normalizePath( void );
	int	sendResponse(const char *resp);
	int	httpDelete( void );
	int	httpGet( void );
	void	sendHeader( void );
	bool	isComplete( void );
	bool	isError( void );

};

#endif
