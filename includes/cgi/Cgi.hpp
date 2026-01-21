#ifndef CGI_HPP
# define CGI_HPP

#include "../request/request.hpp"
#include "../response/ResolvedConfig.hpp"
#include "envUtils.hpp"

class Cgi{
		
		ResolvedConfig*	conf;
		HttpParser*			request;
		char**			env;
		std::string uriPath;

	public:
		Cgi( void );
		~Cgi( void );

		//env creation
		std::map<std::string, std::string>	mapEnvBuilder( void );
		char**															charEnvBuilder( std::map<std::string, std::string> );
		void																envBuilder( void );
		
		//execution
		void executor( void );
		


};

#endif