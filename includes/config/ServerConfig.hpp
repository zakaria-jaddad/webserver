#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include "LocationConfig.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <vector>

class ServerConfig{
  
  std::map<int, std::string>  error_pages;
  std::vector<std::string>    index;
  std::string                 ip;
  std::string                 server_name;
  std::string                 root;
  std::size_t                 max_body;
  int                         port;
  
  typedef void (ServerConfig::*Parser)(std::vector<std::string>&, size_t&);
  std::map<std::string, Parser> serverParsers;
  
  public:
  std::vector<LocationConfig> location;
  ServerConfig();

  void buildServerParsers( void );

  void parseBlock( std::vector<std::string>&, size_t& );

  void parseName( std::vector<std::string>&, size_t& );
  void checkServerName( const std::string& );

  void parseListen( std::vector<std::string>&, size_t& );
  void parseIpOnly( const std::string& );
  void parseIpPort( const std::string& );
  void parsePortOnly( const std::string& );


  void parseIndex( std::vector<std::string>&, size_t& );
  
  void parseError( std::vector<std::string>&, size_t& );
  
  void parseSize( std::vector<std::string>&, size_t& );
  
  void parseRoot( std::vector<std::string>&, size_t& );

  std::map<std::string, int>  getListeningPairs( void );
  std::map<int, std::string>  getErrorPages( void );
  std::vector<std::string>    getIndex( void );
  std::size_t                 getSize( void );
  std::string                 getServerName( void );
  std::string                 getRoot( void );
  std::string                 getIP( void );
  int			      getPort( void );
};

#endif
