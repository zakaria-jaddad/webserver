#ifndef CONFIG_HPP
# define CONFIG_HPP

#include "ServerConfig.hpp"
#include <iostream>


class Config{
  
  std::vector<std::string> tokens;//dont forget to make it private

public:
  std::vector<ServerConfig> servers;//don't forget to make it private
  void tokenize( const std::string&  );
  void loadServers( void );

};

#endif
