#include "../../includes/config/Config.hpp"
#include <string>

int main()
{
  Config conf;

  conf.tokenize("configExample.conf");
  conf.loadServers();

 /* for (std::vector<std::string>::iterator it = conf.tokens.begin(); it != conf.tokens.end(); ++it) {
    std::cout << *it << std::endl;
  }*/

}
