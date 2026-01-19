#ifndef  LOCATIONCONFIG_HPP 
# define LOCATIONCONFIG_HPP

#include <cstddef>
#include <map>
#include <string>
#include <vector>

class LocationConfig{
	
	std::map<std::string, std::string>  cgi_info; //don't forget to check if this is how it should be handled
	std::vector<std::string>    index;
	std::vector<std::string>	methods_accepted;
	std::map<int, std::string>  redir; //don't forget to check if this is how it should be handled
	std::string                 root;
	std::string                 upload_path;
	std::string                 location_path;
	bool                        directory_listing;//only a fallback when index is not specified

	typedef void (LocationConfig::*Parser)(std::vector<std::string>&, size_t&);
	std::map<std::string, Parser> locationParsers;

	void buildLocationParsers( void );

public:
	LocationConfig( void );

	void parseBlock( std::vector<std::string>&, size_t& );

	void parseRoot( std::vector<std::string>&, size_t& );
	void parseIndex( std::vector<std::string>&, size_t& );
	void parsePath( std::vector<std::string>&, size_t& );
	void parseListing( std::vector<std::string>&, size_t& );
	void parseRedir( std::vector<std::string>&, size_t& );
	void parseMethods( std::vector<std::string>&, size_t& );
	void parseCgi( std::vector<std::string>&, size_t& );

	std::vector<std::string>	getIndex( void );
	std::vector<std::string>	getAcceptedMethods( void );
	std::map<int, std::string>	getRedir( void );
	std::string 				getRoot( void );
	std::string 				getUploadPath( void );
	bool 						getDirectoyListing( void );
	std::string					getLocationPath( void );
	std::map<std::string, std::string>		getCgi( void );

};

#endif
