#ifndef CGI_HPP
# define CGI_HPP

#include <unistd.h>
#include <sys/wait.h>

#include "../request/request.hpp"
#include "../response/ResolvedConfig.hpp"
#include "envUtils.hpp"

class Cgi{

		typedef enum {
			HEADERS,
			BODY,
			COMPLETE,
			ERROR,
		} phase;
		
		int fds[2];
		ResolvedConfig*	conf;
		HttpParser*			request;
		char**			env;
		std::string uriPath;
		phase				_phase = HEADERS;

	public:
		Cgi( void );
		~Cgi( void );

		//env creation
		std::map<std::string, std::string>	mapEnvBuilder(	std::map<std::string, std::vector<std::string>>&,
                                                      	std::string ,
                                                        std::string ,
                                                        std::map<std::string, std::string>& );
		
		char**															charEnvBuilder(	std::map<std::string, std::string> );
		void																envBuilder( void );
		
		//execution
		void executor( void );
		
		int* getPipe( void );

		void craftHeader( std::string&  );
		void craftBody( std::string&  );


};

#endif