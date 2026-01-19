#include "../../includes/response/ResolvedConfig.hpp"

ResolvedConfig::ResolvedConfig( const std::string& uri , ServerConfig* srvr )
{
    LocationConfig* loc = srvr->matchLocation(uri);

    if (loc)
    {
        this->directory_listing = loc->getDirectoyListing();
        this->index = loc->getIndex();
        this->methods_accepted = loc->getAcceptedMethods();
        this->redir = loc->getRedir();
        this->root = loc->getRoot();
        this->cgi_info = loc->getCgi();
        // this->upload_path = loc->getUploadPath();
    }
    else
    {
        this->directory_listing = srvr->getDirectoyListing();
        this->index = srvr->getIndex();
        this->methods_accepted = srvr->getAcceptedMethods();
        this->redir = srvr->getRedir();
        this->root = srvr->getRoot();
        this->cgi_info = srvr->getCgi();
        // this->upload_path = srvr->getUploadPath();

    }
    this->error_pages = srvr->getErrorPages();
}

std::string ResolvedConfig::getRoot( void )
{
  return root;
}

std::map<int, std::string> ResolvedConfig::getErrorPages( void )
{
  return error_pages;
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

std::map<std::string, std::string>	ResolvedConfig::getCgi( void )
{
  return cgi_info;
}