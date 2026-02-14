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
	int							cgi_in_pipe[2];
	int							cgi_out_pipe[2];
	phase						_phase;
	int							fileFD;
	size_t						chunk_size;
	int							headerFlag;
	bool						status;
	bool						exec_status;
	bool						error;

public:
	Response( t_client* client );

	HttpParser::httpResult	checkMethod( void );
	void normalizePath( void );
  std::string resolveFilePath( std::string& );
//   int   createUploadFolder( std::string&, std::string& );
	int	sendResponse(const char *resp);

  //static methods
	int	httpDelete( void );
	int	httpGet( void );
	int httpPost( void );

  void	sendHeader( int );
	bool	isComplete( void );
	bool	isError( void );

	void	craftResponse( void );
	std::string craftHeaderStatus( std::string&  );
	void	craftHeader( std::string& );
	void 	craftBody( std::string& );

	HttpParser::httpResult	cgiExecution();
	void		sendingCgiResponse();

	int 		servePages( int );
	std::string getStatusMessage( int );
	void		craftPageResponse( int, int );

};

#endif
