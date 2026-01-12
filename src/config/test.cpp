#include "../../includes/config/Config.hpp"
#include <map>
#include <ostream>
#include <string>

int main()
{
  Config conf;

  conf.tokenize("configExample.conf");
  conf.loadServers();

  //printing ip:port
  std::map<std::string, int>::iterator m = conf.servers[0].getListeningPairs().begin();
  std::cout << "ip: " <<  m->first << " port : " << m->second << std::endl;

  //printinf server name
  std::cout << "server name : " << conf.servers[0].getServerName() << std::endl;
  

  std::cout << "root : " << conf.servers[0].getRoot() << std::endl;

}
