#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "../server/Server.hpp"
#include "../request/Request.hpp"
#include "ResolvedConfig.hpp"
#include "../cgi/Cgi.hpp"
#include <stdio.h>
#include <algorithm>

#define CGI_TIMEOUT 10

class Cgi;
struct t_client;

//mime
struct MIME {
public:
	MIME();
	std::string getMIME(std::string extension);
	std::map<std::string, std::string> _mime;
};


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
	bool						timed_out;

	time_t 						start_time;

	std::string generateDirectoryListingHTML(const std::string &path, const std::string &requestPath,
                                         std::vector<std::string> entries);

public:
	Response( t_client* client );
	~Response( void );

	HttpParser::httpResult	checkMethod( void );
	HttpParser::httpResult	routeMethod( HttpParser::requestType );
	
	void					normalizePath( void );
	std::string 			normalizePath( std::string& );

  	
	ssize_t	sendResponse(const char *resp, size_t );
//   int   createUploadFolder( std::string&, std::string& );

  //static methods
	HttpParser::httpResult	httpDelete( void );
	HttpParser::httpResult	httpGet( void );
	HttpParser::httpResult	httpPost( void );

	std::string chooseIndex( std::string& );
  void	sendHeader( int, std::string, std::string, bool, bool );
  bool	isComplete( void );
  bool	isError( void );
  
  HttpParser::httpResult craftResponse( void );
  std::string craftHeaderStatus( std::string&  );
  void	craftHeader( std::string& );
  void 	craftBody( std::string& );
  
  HttpParser::httpResult	cgiExecution();
  void		sendingCgiResponse();
  
  int 		servePages( int, bool );
  std::string getStatusMessage( int );
  void		craftPageResponse( int, int, bool );
  
  int 		servePages( int, std::string&, bool );
  void		craftPageResponse( int, std::string&, bool );

  void craftChunkSize( size_t );
  void serveDirectoryContent( std::string& );

};

#endif
