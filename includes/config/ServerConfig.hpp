#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include "LocationConfig.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <vector>

class ServerConfig{
  
  std::map<int, std::string>  error_pages;
  std::string                 ip;
  std::string                 server_name;
  std::string                 root;
  std::string                 index;
  std::size_t                 max_body;
  int                         port;
  //std::vector<LocationConfig> location;
  
public:
  void parseBlock( std::vector<std::string>, size_t& );
};

#endif
