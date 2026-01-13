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

  //printing root directory
  std::cout << "root : " << conf.servers[0].getRoot() << std::endl;
  
  //printing index
  const std::vector<std::string>& indexes = conf.servers[0].getIndex();
  std::vector<std::string>::const_iterator id_it = indexes.begin();
  for (; id_it != indexes.end(); id_it++)
    std::cout << "index : " << *id_it << std::endl;

  //printing size
  std::cout << "Size : " << conf.servers[0].getSize() << std::endl;

  //printing errorpages
  const std::map<int, std::string>& errors = conf.servers[0].getErrorPages();
  std::map<int, std::string>::const_iterator it = errors.begin();
  for (; it != errors.end(); ++it)
    std::cout << "error " << it->first << " -> " << it->second << std::endl;
}
