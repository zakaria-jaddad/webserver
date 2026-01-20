#ifndef CGI_HPP
# define CGI_HPP

#include "../request/request.hpp"
#include "../response/ResolvedConfig.hpp"

class Cgi{
		
		ResolvedConfig*	conf;
		HttpParser*			request;
		char**			env;

	public:
		Cgi( void );
		~Cgi( void );

		//env creation
		std::map<std::string, std::string>	mapEnvBuilder( void );
		std::string 												normalizeHeaderName( const std::string& );
		std::string													joinHeaderValues( std::vector<std::string>& );
		void																headerEnvBuilder( std::map<std::string, std::string>&, std::map<std::string, std::vector<std::string>>& );
		char**															charEnvBuilder( std::map<std::string, std::string> );
		void																envBuilder( void );
		
		//execution
		void executor( void );
		


};

#endif