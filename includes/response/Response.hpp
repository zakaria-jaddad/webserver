#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "../server/Server.hpp"
#include "ResolvedConfig.hpp"
#include "../cgi/Cgi.hpp"

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
	t_client				*client;
	bool						wait;
	Cgi							*cgi;
	int							*cgi_pipe;
	phase						_phase;

public:
	Response( t_client* client );

	//HttpParser::httpResult	checkMethod( void );
	void normalizePath( void );
	int	httpDelete( void );

	void	craftResponse( void );
	std::string craftHeaderStatus( std::string&  );
	void	craftHeader( std::string& );
	void 	craftBody( std::string& );

};

#endif
