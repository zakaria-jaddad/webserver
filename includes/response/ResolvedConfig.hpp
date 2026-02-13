#ifndef RESOLVEDCONFIG_HPP
# define RESOLVEDCONFIG_HPP

#include "../config/ServerConfig.hpp"
#include <map>
#include <string>
#include <vector>

class ResolvedConfig{
	std::map<int, std::string>  error_pages;
	std::pair<std::string, std::string>  cgi_info;
	std::map<int, std::string>  redir;
	std::vector<std::string>		methods_accepted;
  std::vector<std::string>		index;
  std::string                 server_name;
  std::string									root;
	std::string									ip;
	int													port;
	bool												directory_listing;
	// std::string									upload_path;

public:
	ResolvedConfig( const std::string&, ServerConfig* ); //passes the uri as a param using the getUriPath

	std::string  getErrorPages( int );
	std::map<int, std::string>	getRedir( void );
  std::vector<std::string>    getIndex( void );
  std::vector<std::string>    getAcceptedMethods( void );
  std::string                 getRoot( void );
  std::string                 getServerName( void );
	bool 						            getDirectoyListing( void );
	int 						            getPort( void );
	std::string			            getIp( void );
	std::pair<std::string, std::string>           getCgi( void );
};

#endif