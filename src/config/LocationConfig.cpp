#include "../../includes/config/LocationConfig.hpp"
#include "../../includes/config/parseHelpers.hpp"

#include <vector>
#include <iostream>
#include <string>
#include <algorithm>

LocationConfig::LocationConfig()
: cgi_info(),
  index(),
  methods_accepted(),
  redir(),
  root(""),
  upload_path(""),
  location_path(""),
  directory_listing(false)
{
  is_set_cgi_info = false;
	is_set_index = false;
	is_set_methods_accepted = false;
	is_set_redir = false;
	is_set_root = false;
	is_set_upload_path = false;
	is_set_location_path = false;
	is_set_directory_listing = false;
    buildLocationParsers();
}

void LocationConfig::buildLocationParsers()
{
  locationParsers["root"] = &LocationConfig::parseRoot;
  locationParsers["index"] = &LocationConfig::parseIndex;
  locationParsers["upload_store"] = &LocationConfig::parsePath;
	locationParsers["autoindex"] = &LocationConfig::parseListing;
	locationParsers["return"] = &LocationConfig::parseRedir;
	locationParsers["allow"] = &LocationConfig::parseMethods;
	locationParsers["cgi"] = &LocationConfig::parseCgi;
}

void LocationConfig::parseBlock(std::vector<std::string>& tokens, size_t& i)
{
  //skip location token
  i++;

  std::string loc_path = tokens[i];
  
  if (!isAbsolutePath(tokens[i++]) || tokens[i++] != "{")
    throw std::runtime_error("Incorrect location block");

  this->location_path = loc_path;

  while (tokens[i] != "}")
  {
    std::map<std::string, Parser>::iterator it = locationParsers.find(tokens[i]);

    if (it == locationParsers.end())
      throw std::runtime_error("Unknown directive: " + tokens[i]);

    Parser parser = it->second;
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
  this->is_set_root = true;
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
  this->is_set_index = true;

  //skip closing ;
  i++;
}


//parsing autoindex directive
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
  
  this->is_set_directory_listing = true;

}


//parsing cgi directive
void LocationConfig::parseCgi( std::vector<std::string>& tokens, size_t& i)
{
  //skip the directive name token
  i++;
//cgi .php /usr/bin/php-cgi
  if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
    throw std::runtime_error("Found an unexpected : " + tokens[i]);

  std::string extension = tokens[i];
  i++;
  
  if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
    throw std::runtime_error("Found an unexpected : " + tokens[i]);
  
  std::string cgi_path = tokens[i];

  //skipping path
  i++;

  if (tokens[i] != ";")
    throw std::runtime_error("Found an unexpected : '" + tokens[i] + "' When expecting ';'");

  i++;
  
  if (extension != ".php" && extension != ".py")
    throw std::runtime_error("Invalid extension : " + extension);
  
  if (!isAbsolutePath(cgi_path))
    throw std::runtime_error("Invalid path : " + cgi_path);

  this->cgi_info = std::make_pair(extension, cgi_path);

  this->is_set_cgi_info = true;
}


//parsing allow directive
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

  this->is_set_methods_accepted = true;
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
    throw std::runtime_error("Invalid number of arguments in \"upload_store\" directive");

  //skips closing ;
  i++;
  
  if (!isAbsolutePath(value))
    throw std::runtime_error("Invalid upload path : " + value);

  this->upload_path = value;

  this->is_set_upload_path = true;
}


//parsing index directive
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
  this->is_set_redir = true;

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

std::string LocationConfig::getLocationPath( void )
{
  return location_path;
}

std::pair<std::string, std::string>	LocationConfig::getCgi( void )
{
  return cgi_info;
}
