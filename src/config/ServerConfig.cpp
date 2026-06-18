#include "../../includes/config/ServerConfig.hpp"
#include "../../includes/config/parseHelpers.hpp"
#include <iostream>

// #include <cctype>
// #include <cstddef>
// #include <iostream>
// #include <map>
// #include <stdexcept>
// #include <vector>

ServerConfig::ServerConfig()
: redir(),
  cgi_info(),
  index(),
  methods_accepted(),
  server_name(),
  ip("0.0.0.0"),
  root(""),
  max_body(1024),
  directory_listing(false),
  port(80),
  serverParsers()
{
  init_error_pages();
  buildServerParsers();
}

void ServerConfig::init_error_pages()
{
  error_pages[200] = "pages/200_delete.html";
  error_pages[400] = "pages/400.html";
  error_pages[403] = "pages/403.html";
  error_pages[404] = "pages/404.html";
  error_pages[405] = "pages/405.html";
  error_pages[413] = "pages/413.html";
  error_pages[408] = "pages/408.html";
  error_pages[500] = "pages/500.html";
  error_pages[501] = "pages/501.html";
  error_pages[504] = "pages/504.html";
  error_pages[505] = "pages/505.html";
}

void ServerConfig::buildServerParsers()
{
  serverParsers["listen"] = &ServerConfig::parseListen;
  serverParsers["server_name"] = &ServerConfig::parseName;
  serverParsers["root"] = &ServerConfig::parseRoot;
  serverParsers["index"] = &ServerConfig::parseIndex;
  serverParsers["error_page"] = &ServerConfig::parseError;
  serverParsers["client_max_body_size"] = &ServerConfig::parseSize;
	serverParsers["autoindex"] = &ServerConfig::parseListing;
	serverParsers["return"] = &ServerConfig::parseRedir;
	serverParsers["allow"] = &ServerConfig::parseMethods;
	serverParsers["cgi"] = &ServerConfig::parseCgi;
}


//parsing
void ServerConfig::parseBlock(std::vector<std::string>& tokens, size_t& i)
{
  if (tokens[i++] != "server" || tokens[i++] != "{")
    throw std::runtime_error("Missing server block");

  while (tokens[i] != "}")
  {
    if (tokens[i] == "location")
    {
      LocationConfig loc;
      loc.parseBlock(tokens, i);
      location.push_back(loc);
    }
    else
    {
      std::map<std::string, Parser>::iterator it = serverParsers.find(tokens[i]);

      if (it == serverParsers.end())
        throw std::runtime_error("Unknown directive: " + tokens[i]);

      Parser parser = it->second;
      (this->*parser)(tokens, i);
    }
  }
  i++;
}


//parsing autoindex directive
void ServerConfig::parseListing( std::vector<std::string>& tokens, size_t& i)
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


//parsing allow directive
void ServerConfig::parseMethods( std::vector<std::string>& tokens, size_t& i)
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


//return directive parsing
void ServerConfig::parseRedir( std::vector<std::string>& tokens, size_t& i)
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
  
  this->redir = std::make_pair(number, path);

  //skip closing ;
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
void ServerConfig::checkServerName( const std::string& name )
{
  std::string val;

  for (size_t i = 0; i < name.size(); ++i)
  {
    unsigned char c = static_cast<unsigned char>(name[i]);
    if (!std::isalnum(c) && c != '-' && c != '.')
      throw std::runtime_error("Invalid server name : " + name);

    val.push_back(std::tolower(c));
  }
  this->server_name.push_back(val);
}

void ServerConfig::parseName( std::vector<std::string>& tokens, size_t& i)
{
  //skip the directive name token
  i++;
  
  if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
    throw std::runtime_error("Found an unexpected : " + tokens[i]);
  
  // std::string value = tokens[i];
  
  // //skip the value token
  // i++;

  // if (tokens[i] != ";")
  //   throw std::runtime_error("Invalid number of argumenst in \"server_name\" directive");

  //skip the closing ';'
  // i++;

  while(tokens[i] != ";")
  {
    std::string value = tokens[i];
    checkServerName(value);
    // this->index.push_back(value);
    i++;
  }
  
  i++;
}


//parsing root directive
void ServerConfig::parseRoot( std::vector<std::string>& tokens, size_t& i)
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
void ServerConfig::parseIndex( std::vector<std::string>& tokens, size_t& i)
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


//parsing error_pages
void ServerConfig::parseError( std::vector<std::string>& tokens, size_t& i)
{
  //skip the directive name token
  i++;

  if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
    throw std::runtime_error("Found an unexpected : " + tokens[i]);

  std::vector<int> codes;
  while(tokens[i] != ";")
  {
    if (tokens[i] == "{" || tokens[i] == "}")
      throw std::runtime_error("Unexpected token in error_page: " + tokens[i]);

    codes.push_back(std::atoi(tokens[i].c_str()));
    i++;
  }

  if (codes.size() < 2)
    throw std::runtime_error("error_page requires at least one code and a path");

  std::string path = tokens[i - 1];

  if (path.empty() || path[0] != '/')
    throw std::runtime_error("error_page path must start with '/'");

  for (size_t j = 0; j < codes.size() - 1; j++)
  {
    int code = codes[j];

    if (code < 400 || code > 599)
      throw std::runtime_error("Invalid HTTP error code");

    error_pages[code] = path;
  }

  //skip closing ;
  i++;
}


//parsing max body size
std::size_t checkSize( std::string& value )
{
  if (value.empty())
    throw std::runtime_error("client_max_body_size: empty value");

  char suffix = value[value.size() - 1];
  std::size_t multiplier = 1;
  std::string numberPart = value;

  if (!std::isdigit(static_cast<unsigned char>(suffix)))
  {
    if (suffix == 'k' || suffix == 'K')
      multiplier = 1024;
    else if (suffix == 'm' || suffix == 'M')
      multiplier = 1024 * 1024;
    else if (suffix == 'g' || suffix == 'G')
      multiplier = 1024 * 1024 * 1024;
    else
      throw std::runtime_error("client_max_body_size: invalid suffix");

    numberPart = value.substr(0, value.size() - 1);
  }

  if (numberPart.empty())
    throw std::runtime_error("client_max_body_size: invalid value");

  if (!isNumber(numberPart))
    throw std::runtime_error("client_max_body_size: invalid number");

  std::size_t size = static_cast<std::size_t>(std::atoi(numberPart.c_str()));

  if (size == 0)
    throw std::runtime_error("client_max_body_size: must be greater than zero");

  return size * multiplier;
}

void ServerConfig::parseSize( std::vector<std::string>& tokens, size_t& i)
{
  //skip the directive name token
  i++;

  if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
    throw std::runtime_error("Found an unexpected : " + tokens[i]);

  std::string value = tokens[i];
  //skip the value token
  i++;

  if (tokens[i] != ";")
    throw std::runtime_error("Invalid number of argumenst in \"client_max_body_size\" directive");

  //skip the closing ';'
  i++;
  
  this->max_body = checkSize(value);
}


//parsing cgi directive
void ServerConfig::parseCgi( std::vector<std::string>& tokens, size_t& i)
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
}


//utility methods
LocationConfig* ServerConfig::matchLocation( const std::string& uri )
{
  //Method used to match uri to the best matching location block
  //it returns a pointetr to a location object if a match was found or NULL otherwise


  LocationConfig* best = NULL;
  std::size_t     bestlen = 0;

  for (size_t i = 0; i < location.size(); i++)
  {
    std::string loc = location[i].getLocationPath();
    
    if (uri.compare(0, loc.size(), loc) == 0)
    {
      if (loc == "/" || loc.size() == uri.size() || uri[loc.size()] == '/')
      {
        if (loc.size() > bestlen)
        {
          bestlen = loc.size();
          best = &location[i];
        }
      }
    }
  }
  
  return best;
}


//getters
std::map<std::string, int> ServerConfig::getListeningPairs()
{
  std::map<std::string, int> m;
  m[this->ip] = this->port;
  return m;
}

std::vector<std::string> ServerConfig::getServerName( void )
{
  return server_name;
}

std::string ServerConfig::getRoot( void )
{
  return root;
}

std::map<int, std::string> ServerConfig::getErrorPages( void )
{
  return error_pages;
}

std::vector<std::string> ServerConfig::getIndex( void )
{
  return index;
}

std::size_t ServerConfig::getSize( void )
{
  return max_body;
}

std::string ServerConfig::getIP ( void ) {
	return ip;
}

int ServerConfig::getPort ( void ) {
	return port;
}

bool  ServerConfig::getDirectoyListing( void )
{
  return directory_listing;
}

std::vector<std::string>	ServerConfig::getAcceptedMethods( void )
{
  return methods_accepted;
}

std::pair<int, std::string>	ServerConfig::getRedir( void )
{
  return redir;
}

std::pair<std::string, std::string>	ServerConfig::getCgi( void )
{
  return cgi_info;
}
