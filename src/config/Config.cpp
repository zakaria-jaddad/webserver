#include "../../includes/config/Config.hpp"
#include "../../includes/config/parseHelpers.hpp"

void Config::tokenize(const std::string& path)
{
  std::string input = readFile(path);
  std::string current;
  bool in_comment = false;
  int in = 0;

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
      if (c == '{')
        in++;
      if (c == '}')
        in--;
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

  if (in > 0)
    throw std::runtime_error("Unmatched \'{\'");

  if (in < 0)
    throw std::runtime_error("Unmatched \'}\'");

  if (!current.empty())
    tokens.push_back(current);
}

void Config::loadServers()
{
  size_t i = 0;

  while (i < tokens.size())
  {
    ServerConfig server;
    server.parseBlock(tokens, i);
    servers.push_back(server);
  }
}
