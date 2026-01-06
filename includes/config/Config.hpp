#ifndef CONFIG_HPP
# define CONFIG_HPP

#include "ServerConfig.hpp"
#include <iostream>

class Config{
  
    std::vector<ServerConfig> servers;
  public:
    void LoadServers( std::string );

};


#endif
