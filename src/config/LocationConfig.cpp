#include "../../includes/config/LocationConfig.hpp"

#include <vector>
#include <iostream>

void LocationConfig::parseBlock(std::vector<std::string>& tokens, size_t& i)
{
  while(tokens[i] != "}")
  {
    std::cout << "parsing location directive : " << tokens[i] << std::endl;
    i++;
  }
  i++;
}
