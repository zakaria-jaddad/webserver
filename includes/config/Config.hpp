#ifndef CONFIG_HPP
# define CONFIG_HPP

#include "ServerConfig.hpp"
#include <iostream>
#include <string>
#include <vector>

class Config{
 
    std::vector<ServerConfig> servers;
  public:
    std::vector<std::string> tokens;//dont forget to make it private
    void tokenize( const std::string&  );
    void loadServers( void );

};


#endif
