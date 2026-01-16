#include "../../includes/config/LocationConfig.hpp"
#include "../../includes/config/parseHelpers.hpp"

#include <vector>
#include <iostream>
#include <string>
#include <algorithm>

LocationConfig::LocationConfig()
    :  index(),
      methods_accepted(),
      redir(),
      root(""),
      upload_path(""),
     directory_listing(false)
{
    buildLocationParsers();
}

void LocationConfig::buildLocationParsers()
{
  locationParsers["root"] = &LocationConfig::parseRoot;
  locationParsers["index"] = &LocationConfig::parseIndex;
  locationParsers["client_body_temp_path"] = &LocationConfig::parsePath;
	locationParsers["autoindex"] = &LocationConfig::parseListing;
	locationParsers["return"] = &LocationConfig::parseRedir;
	locationParsers["allow"] = &LocationConfig::parseMethods;
}

void LocationConfig::parseBlock(std::vector<std::string>& tokens, size_t& i)
{
  //skip location token
  i++;

  if (tokens[i++].find('/') == std::string::npos || tokens[i++] != "{")
    throw std::runtime_error("Incorrect location block");

  while (tokens[i] != "}")
  {
    std::map<std::string, Parser>::iterator it = locationParsers.find(tokens[i]);

    if (it == locationParsers.end())
      throw std::runtime_error("Unknown directive: " + tokens[i]);

    Parser parser = it->second;
    std::cout << "parsing : " << it->first << std::endl;
    (this->*parser)(tokens, i);
  }
  i++;
}


//parsing root directive
void LocationConfig::parseRoot( std::vector<std::string>& tokens, size_t& i)
{
  //skip the directive name token
  i++;
  
  if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
    throw std::runtime_error("Found an unexpected : " + tokens[i]);
  
  std::string value = tokens[i];
  
  //skip the value token
  i++;

  if (tokens[i] != ";")
    throw std::runtime_error("Invalid number of argumenst in \"root\" directive");

  //skip the closing ';'
  i++;
  
  for (size_t i = 0; i < value.size(); ++i)
  {
    if (!std::isalnum(value[i]) && value[i] != '-' && value[i] != '.' && value[i] != '/' && value[i] != '_')
      throw std::runtime_error("Invalid root directory : " + value);
  }
  this->root = value;
}


//parsing index directive
void LocationConfig::parseIndex( std::vector<std::string>& tokens, size_t& i)
{
  //skip the directive name token
  i++;

  if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
    throw std::runtime_error("Found an unexpected : " + tokens[i]);

  while(tokens[i] != ";")
  {
    std::string value = tokens[i];
    for (size_t y = 0; y < value.size(); y++)
    {
      if (!std::isalnum(value[y]) && value[y] != '-' && value[y] != '.' && value[y] != '_')
        throw std::runtime_error("Invalid index  : " + value);
    }
    this->index.push_back(value); 
    i++;
  }

  //skip closing ;
  i++;
}


//parsing index directive
void LocationConfig::parseListing( std::vector<std::string>& tokens, size_t& i)
{
  //skip the directive name token
  i++;

  if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
    throw std::runtime_error("Found an unexpected : " + tokens[i]);

  std::string value = tokens[i];

  //skips value
  i++;
  
  if (tokens[i] != ";")
    throw std::runtime_error("Invalid number of argumenst in \"autoindex\" directive");

  //skips closing ;
  i++;

  if (value == "on")
    this->directory_listing = true;
  else if (value == "off")
    this->directory_listing = false;
  else
    throw std::runtime_error("Invalid value for : \"autoindex\" directive : " + value);

}


//parsing index directive
void LocationConfig::parseMethods( std::vector<std::string>& tokens, size_t& i)
{
  //skip the directive name token
  i++;

  if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
    throw std::runtime_error("Found an unexpected : " + tokens[i]);

  std::string methods[] = {"GET", "POST", "DELETE", "HEAD", "PUT", "OPTIONS" ,"TRACE", "CONNECT"};

  while(tokens[i] != ";")
  {
    std::string value = tokens[i];
    if (std::find(methods, methods + 8, value) == (methods + 8))
      throw std::runtime_error("Found an unexpected method : " + value);
    this->methods_accepted.push_back(value);
    i++;
  }

  //skip closing ;
  i++;
}


//parsing index directive
void LocationConfig::parsePath( std::vector<std::string>& tokens, size_t& i)
{
  //skip the directive name token
  i++;

  if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
    throw std::runtime_error("Found an unexpected : " + tokens[i]);

  std::string value = tokens[i];

  //skips value
  i++;
  
  if (tokens[i] != ";")
    throw std::runtime_error("Invalid number of arguments in \"client_body_temp_path\" directive");

  //skips closing ;
  i++;
  
  if (!isAbsolutePath(value))
    throw std::runtime_error("Invalid upload path : " + value);

  this->upload_path = value;
}


//parsing index directive
bool isRedirectCode(int num)
{
  return (num == 301 || num == 302 || num == 303 || num == 307 || num == 308);
}

void LocationConfig::parseRedir( std::vector<std::string>& tokens, size_t& i)
{
  //skip the directive name token
  i++;

  std::string value = tokens[i];

  //skipping value
  i++;

  if (value == ";" || value == "{" || value == "}" ||
      tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
    throw std::runtime_error("Found an unexpected : " + value);

  std::string path = tokens[i];

  //skipping path
  i++;

  if (!isNumber(value) || value.size() != 3 || !isRedirectCode(std::atoi(value.c_str())))
    throw std::runtime_error("Invalid return code");

  if (!isAbsolutePath(path))
    throw std::runtime_error("Invalid redirection path");
  
  int number = std::atoi(value.c_str());
  
  this->redir[number] = path;

  //skip closing ;
  i++;
}


//getters
std::vector<std::string>	LocationConfig::getIndex( void )
{
  return index;
}

std::vector<std::string>	LocationConfig::getAcceptedMethods( void )
{
  return methods_accepted;
}

std::map<int, std::string>	LocationConfig::getRedir( void )
{
  return redir;
}

std::string LocationConfig::getRoot( void )
{
  return root;
}

std::string LocationConfig::getUploadPath( void )
{
  return upload_path;
}

bool  LocationConfig::getDirectoyListing( void )
{
  return directory_listing;
}
