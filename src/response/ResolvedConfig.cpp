#include "../../includes/response/ResolvedConfig.hpp"
#include <iostream>

ResolvedConfig::ResolvedConfig( const std::string& uri , ServerConfig* srvr )
{
  LocationConfig* loc = srvr->matchLocation(uri);

  this->directory_listing = srvr->getDirectoyListing();
  this->index = srvr->getIndex();
  this->methods_accepted = srvr->getAcceptedMethods();
  this->redir = srvr->getRedir();
  this->root = srvr->getRoot();
  this->cgi_info = srvr->getCgi();
  this->upload_path = "";
  this->location_path = "";
  this->ip = srvr->getIP();
  this->port = srvr->getPort();
  this->server_name = srvr->getServerName();
  this->error_pages = srvr->getErrorPages();

  if (loc)
  {
		if (loc->is_set_directory_listing)
    	this->directory_listing = loc->getDirectoyListing();
    if (loc->is_set_index)
			this->index = loc->getIndex();
    if(loc->is_set_methods_accepted)
			this->methods_accepted = loc->getAcceptedMethods();
    if (loc->is_set_redir)
			this->redir = loc->getRedir();
    if (loc->is_set_root)
			this->root = loc->getRoot();
    if (loc->is_set_cgi_info)
			this->cgi_info = loc->getCgi();
    if (loc->is_set_upload_path)
			this->upload_path = loc->getUploadPath();
    if (loc->is_set_location_path)
			this->location_path = loc->getLocationPath();
  }

}

std::string ResolvedConfig::getRoot( void )
{
  return root;
}

std::string ResolvedConfig::getErrorPages( int status )
{
  return error_pages[status];
}

std::vector<std::string> ResolvedConfig::getIndex( void )
{
  return index;
}

bool  ResolvedConfig::getDirectoyListing( void )
{
  return directory_listing;
}

std::vector<std::string>	ResolvedConfig::getAcceptedMethods( void )
{
  return methods_accepted;
}

std::map<int, std::string>	ResolvedConfig::getRedir( void )
{
  return redir;
}

std::pair<std::string, std::string>	ResolvedConfig::getCgi( void )
{
  return cgi_info;
}

std::string ResolvedConfig::getServerName( void )
{
  return server_name;
}

std::string ResolvedConfig::getIp( void )
{
  return ip;
}

int ResolvedConfig::getPort( void )
{
  return port;
}

std::string ResolvedConfig::getUploadPath()
{
  return upload_path;
}

std::string ResolvedConfig::getLocPath()
{
  return location_path;
}
