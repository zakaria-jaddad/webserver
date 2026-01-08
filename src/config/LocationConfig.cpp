#include "../../includes/config/LocationConfig.hpp"

#include <vector>
#include <iostream>

void LocationConfig::parseBlock(std::vector<std::string>& tokens, size_t& i)
{
  i+=2;
  while (tokens[i] != "}")
  {
    std::cout << tokens[i] << std::endl;
    i++;
  }
  i++;
}
