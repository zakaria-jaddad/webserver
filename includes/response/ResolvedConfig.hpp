#ifndef RESOLVEDCONFIG_HPP
# define RESOLVEDCONFIG_HPP

#include "../config/ServerConfig.hpp"
#include <map>
#include <string>
#include <vector>

class ResolvedConfig{
	std::pair<std::string, std::string>	cgi_info;
	std::map<int, std::string>					error_pages;
	std::map<int, std::string>  				redir;
	std::vector<std::string>						methods_accepted;
  std::vector<std::string>						index;
  std::string                 				server_name;
  std::string													root;
	std::string													ip;
	std::string													upload_path;
  std::string                 				location_path;
	int																	port;
	bool																directory_listing;

public:
	ResolvedConfig( const std::string&, ServerConfig* ); //passes the uri as a param using the getUriPath

	std::map<int, std::string>	getRedir( void );
  std::vector<std::string>    getIndex( void );
  std::vector<std::string>    getAcceptedMethods( void );
	std::string  								getErrorPages( int );
  std::string                 getRoot( void );
  std::string                 getServerName( void );
	bool 						            getDirectoyListing( void );
	int 						            getPort( void );
	std::string			            getIp( void );
	std::string			            getUploadPath( void );
	std::string			            getLocPath( void );
	std::pair<std::string, std::string>           getCgi( void );
};

#endif
