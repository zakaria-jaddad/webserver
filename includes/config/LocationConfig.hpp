#ifndef  LOCATIONCONFIG_HPP 
# define LOCATIONCONFIG_HPP

#include <cstddef>
#include <map>
#include <string>
#include <vector>

class LocationConfig{
	
	std::pair<std::string, std::string>  cgi_info;
	std::vector<std::string>    index;
	std::vector<std::string>	methods_accepted;
	std::map<int, std::string>  redir;
	std::string                 root;
	std::string                 upload_path;
	std::string                 location_path;
	bool                        directory_listing;

	
	typedef void (LocationConfig::*Parser)(std::vector<std::string>&, size_t&);
	std::map<std::string, Parser> locationParsers;
	
	void buildLocationParsers( void );
	
public:

	bool	is_set_cgi_info;
	bool	is_set_index;
	bool	is_set_methods_accepted;
	bool	is_set_redir;
	bool	is_set_root;
	bool	is_set_upload_path;
	bool	is_set_location_path;
	bool	is_set_directory_listing;
	LocationConfig( void );
	
	void parseBlock( std::vector<std::string>&, size_t& );

	void init_error_pages();

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
	std::pair<std::string, std::string>		getCgi( void );

};

#endif
