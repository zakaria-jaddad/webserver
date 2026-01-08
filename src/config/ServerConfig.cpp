#include "../../includes/config/ServerConfig.hpp"
#include <stdexcept>
#include <vector>
#include <iostream>

ServerConfig::ServerConfig()
  : error_pages(),
    ip("0.0.0.0"),
    server_name(""),
    root(""),
    index(""),
    max_body(0),
    port(80),
    serverParsers()
{
    buildServerParsers();
}

void ServerConfig::buildServerParsers()
{
  serverParsers["listen"] = &ServerConfig::parseListen;
  serverParsers["server_name"] = &ServerConfig::parseName;
  serverParsers["root"] = &ServerConfig::parseRoot;
  serverParsers["index"] = &ServerConfig::parseIndex;
  serverParsers["error_page"] = &ServerConfig::parseError;
  serverParsers["client_max_body_size"] = &ServerConfig::parseSize;
}

void ServerConfig::parseBlock(std::vector<std::string>& tokens, size_t& i)
{
  if (tokens[i++] != "server" || tokens[i++] != "{")
    throw std::runtime_error("Missing server block");

  while (tokens[i] != "}")
  {
    std::map<std::string, Parser>::iterator it = serverParsers.find(tokens[i]);

    if (it == serverParsers.end())
      throw std::runtime_error("Unknown directive: " + tokens[i]);

    Parser parser = it->second;
    (this->*parser)(tokens, i);
  }
  i++;
}

void ServerConfig::parseListen( std::vector<std::string>& tokens, size_t& i)
{
  while (tokens[i] != ";")
  {
    std::cout << tokens[i] << std::endl;
    i++;
  }
  i++;
}
void ServerConfig::parseName( std::vector<std::string>& tokens, size_t& i)
{
  while (tokens[i] != ";")
  {
    std::cout << tokens[i] << std::endl;
    i++;
  }
  i++;
}
void ServerConfig::parseIndex( std::vector<std::string>& tokens, size_t& i)
{
  while (tokens[i] != ";")
  {
    std::cout << tokens[i] << std::endl;
    i++;
  }
  i++;
}

void ServerConfig::parseError( std::vector<std::string>& tokens, size_t& i)
{
  while (tokens[i] != ";")
  {
    std::cout << tokens[i] << std::endl;
    i++;
  }
  i++;
}

void ServerConfig::parseRoot( std::vector<std::string>& tokens, size_t& i)
{
  while (tokens[i] != ";")
  {
    std::cout << tokens[i] << std::endl;
    i++;
  }
  i++;
}

void ServerConfig::parseSize( std::vector<std::string>& tokens, size_t& i)
{
  while (tokens[i] != ";")
  {
    std::cout << tokens[i] << std::endl;
    i++;
  }
  i++;
}
