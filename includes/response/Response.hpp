#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "../server/Server.hpp"
#include "../request/Request.hpp"
#include "ResolvedConfig.hpp"
#include "../cgi/Cgi.hpp"
#include <stdio.h>

class Cgi;
struct t_client;

class Response{

	typedef enum {
		REQUEST_LINE,
		HEADERS,
		BODY,
		COMPLETE,
		ERROR,
	} phase;

	ResolvedConfig	conf;
	std::string			normalizedPath;
	std::string			resp;
	std::string			head_resp;
	t_client				*client;
	bool						wait;
	Cgi							*cgi;
	int							*cgi_in_pipe;
	int							*cgi_out_pipe;
	phase						_phase;
	int							fileFD;
	size_t						chunk_size;
	int							headerFlag;
	bool						status;
	bool						error;

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

	void	craftResponse( void );
	std::string craftHeaderStatus( std::string&  );
	void	craftHeader( std::string& );
	void 	craftBody( std::string& );

	HttpParser::httpResult	cgiExecution();
	void		sendingCgiResponse();

};

#endif
