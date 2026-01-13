#include "../../includes/config/Config.hpp"
#include "../../includes/config/ServerConfig.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <stdexcept>

std::string readFile(const std::string& path)
{
  std::ifstream file(path.c_str());
  if (!file)
    throw std::runtime_error("Cannot open file");

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

void Config::tokenize(const std::string& path)
{
  std::string input = readFile(path);
  std::string current;
  bool in_comment = false;

  for (size_t i = 0; i < input.size(); ++i)
  {
    char c = input[i];

    // Handle comments
    if (in_comment) {
      if (c == '\n')
        in_comment = false;
      continue;
    }

    if (c == '#') {
      in_comment = true;
      continue;
    }

    if (std::isspace(c)) 
    {
      if (!current.empty()) 
      {
        tokens.push_back(current);
        current.clear();
      }
      continue;
    }

    if (c == '{' || c == '}' || c == ';') 
    {
      if (!current.empty()) 
      {
        tokens.push_back(current);
        current.clear();
      }
      std::string s(1, c);
      tokens.push_back(s);
      continue;
    }

    current += c;
  }

  if (!current.empty())
    tokens.push_back(current);
}

void Config::loadServers()
{
  size_t i = 0;

  while (i < tokens.size())
  {
    try {
      ServerConfig server;
      server.parseBlock(tokens, i);
      servers.push_back(server);
    }
    catch(std::exception &e) {
      std::cout << "Parsing error : " << e.what() << std::endl;
      break ;
    }
  }
}
