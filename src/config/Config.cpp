#include "../../includes/config/Config.hpp"
#include <fstream>
#include <string>

void Config::LoadServers( std::string path )
{
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error: could not open file" << std::endl;
    return ;
  }

  std::string line;
  while(std::getline(file, line))
  {
    std::cout << "reading the line : " << line << std::endl;
    std::size_t found = line.find("server");
    if (found != std::string::npos)
    {
      std::cout << "Found a server" << std::endl;
    }
  }
}
