#include "../../includes/config/Config.hpp"
#include <map>
#include <ostream>
#include <string>

int main()
{
  Config conf;

  conf.tokenize("configExample.conf");
  conf.loadServers();

  std::map<std::string, int>::iterator m = conf.servers[0].getListeningPairs().begin();
  std::cout << "ip: " <<  m->first << " port : " << m->second << std::endl;
 /* for (std::vector<std::string>::iterator it = conf.tokens.begin(); it != conf.tokens.end(); ++it) {
    std::cout << *it << std::endl;
  }*/

}
