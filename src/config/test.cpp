#include "../../includes/config/Config.hpp"
#include <map>
#include <ostream>
#include <string>

int main()
{
  Config conf;

  try{
    conf.tokenize("configExample.conf");
    conf.loadServers();
    //printing ip:port
    std::map<std::string, int>::iterator m = conf.servers[0].getListeningPairs().begin();
    std::cout << "ip: " <<  m->first << " port : " << m->second << std::endl;

    //printing server name
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

    const std::vector<std::string>& me_indexes = conf.servers[0].getAcceptedMethods();
    std::vector<std::string>::const_iterator me_it = me_indexes.begin();
    for (; me_it != me_indexes.end(); me_it++)
      std::cout << "allowed : " << *me_it << std::endl;

    //printing error pages
    const std::map<int, std::string>& errors = conf.servers[0].getErrorPages();
    std::map<int, std::string>::const_iterator it = errors.begin();
    for (; it != errors.end(); ++it)
      std::cout << "error " << it->first << " -> " << it->second << std::endl;

    
    //printing location directives
    LocationConfig loc =  conf.servers[0].location[0];

    std::cout << "location directive" << std::endl;
    //printing index
    
    std::cout << "location path : " << loc.getLocationPath() << std::endl;

    const std::vector<std::string>& loc_indexes = loc.getIndex();
    std::vector<std::string>::const_iterator loc_it = loc_indexes.begin();
    for (; loc_it != loc_indexes.end(); loc_it++)
      std::cout << "index : " << *loc_it << std::endl;

    //printing allowed methods
    const std::vector<std::string>& met_indexes = loc.getAcceptedMethods();
    std::vector<std::string>::const_iterator met_it = met_indexes.begin();
    for (; met_it != met_indexes.end(); met_it++)
      std::cout << "allowed : " << *met_it << std::endl;

    std::cout << "root : " << loc.getRoot() << std::endl;
    std::cout << "upload path : " << loc.getUploadPath() << std::endl;
    std::cout << "directory listing : " << loc.getDirectoyListing() << std::endl;

    //printing redirections
    const std::map<int, std::string>& red = loc.getRedir();
    std::map<int, std::string>::const_iterator red_it = red.begin();
    for (; red_it != red.end(); ++red_it)
      std::cout << "error " << red_it->first << " -> " << red_it->second << std::endl;
    

    //testing location matching
    std::string uri = "/images/icons";
    LocationConfig* locat = conf.servers[0].matchLocation(uri);

    std::cout << "\n\nmatching location for request with : \"/images/icons\" --> best path found : " << locat->getLocationPath() << std::endl;
  }
  catch(std::exception &e) {
    std::cout << "Parsing error : " << e.what() << std::endl;
  }
}
