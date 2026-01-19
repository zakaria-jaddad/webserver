#ifndef RESOLVEDCONFIG_HPP
# define RESOLVEDCONFIG_HPP

#include "../config/ServerConfig.hpp"
#include <map>
#include <string>
#include <vector>

class ResolvedConfig{
	std::map<int, std::string>  error_pages;
	std::map<int, std::string>  redir;
	std::vector<std::string>		methods_accepted;
  std::vector<std::string>		index;
  std::string									root;
	bool												directory_listing;
	// std::string									upload_path;

public:
	ResolvedConfig( const std::string&, ServerConfig* ); //passes the uri as a param using the getUriPath

};

#endif