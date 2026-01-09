#include "../../includes/config/ServerConfig.hpp"
#include "../../includes/config/parseHelpers.hpp"

#include <cctype>
#include <map>
#include <stdexcept>
#include <vector>

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


//parsing
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

//listen directive parsing
void ServerConfig::parseIpOnly( const std::string& value)
{
  if (!isValidIPv4(value))
    throw std::runtime_error("listen: invalid IP address");
  this->ip = value;
}

void ServerConfig::parsePortOnly(const std::string& value)
{
  if (!isNumber(value))
    throw std::runtime_error("listen: invalid port");

  int p = std::atoi(value.c_str());
  if (p <= 0 || p > 65535)
    throw std::runtime_error("listen: port out of range");
  
  this->port = p;
}

void ServerConfig::parseIpPort(const std::string& value)
{
  size_t pos = value.find(':');
  std::string ipPart = value.substr(0, pos);
  std::string portPart = value.substr(pos + 1);

  if (!isValidIPv4(ipPart))
    throw std::runtime_error("listen: invalid IP address");

  if (!isNumber(portPart))
    throw std::runtime_error("listen: invalid port");

  int p = std::atoi(portPart.c_str());
  if (p <= 0 || p > 65535)
    throw std::runtime_error("listen: port out of range");

  this->ip = ipPart;
  this->port = p;
}

void ServerConfig::parseListen( std::vector<std::string>& tokens, size_t& i)
{
  //skip the listen token
  i++;
  
  if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
    throw std::runtime_error("Found an unexpected : " + tokens[i]);
  
  std::string value = tokens[i];
  
  //skip the value token
  i++;

  if (tokens[i] != ";")
    throw std::runtime_error("Invalid number of argumenst in \"listen\" directive");

  //skip the closing ';'
  i++;

  if (value.find(":") != std::string::npos)
    parseIpPort(value);
  else if (value.find(".") != std::string::npos)
    parseIpOnly(value);
  else
    parsePortOnly(value);
}


//hostname directive parsing
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

std::map<std::string, int> ServerConfig::getListeningPairs()
{
  std::map<std::string, int> m;
  m[this->ip] = this->port;
  return m;
}
