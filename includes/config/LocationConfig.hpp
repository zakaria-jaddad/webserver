#ifndef  LOCATIONCONFIG_HPP 
# define LOCATIONCONFIG_HPP

#include <cstddef>
#include <map>
#include <string>
#include <vector>

class LocationConfig{
  
  public:
    void parseBlock( std::vector<std::string>&, size_t& );
};

#endif
