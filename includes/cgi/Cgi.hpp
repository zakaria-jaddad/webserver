#ifndef CGI_HPP
# define CGI_HPP

#include <unistd.h>
#include <sys/wait.h>

#include "../request/Request.hpp"
#include "../response/ResolvedConfig.hpp"
#include "../server/Server.hpp"
#include "envUtils.hpp"

struct t_client;

class Cgi{

		// typedef enum {
		// 	HEADERS,
		// 	BODY,
		// 	COMPLETE,
		// 	ERROR,
		// } phase;
		
		ResolvedConfig*	conf;
		HttpParser*			request;
		std::string 		uriPath;
		std::string 		cgiPath;
		std::string 		scriptPath;
		int 						*in_fds;
		int 						*out_fds;
		char**					env;
		pid_t					child;
		int						child_status;

    std::string body_path;
    int body_fd;

    std::string content_type;
    std::string content_length;
		// phase				_phase;

	public:
		Cgi( int *, int * , t_client* );
		~Cgi( void );

		//env creation
		std::map<std::string, std::string>	mapEnvBuilder(	std::map<std::string, std::vector<std::string> >&,
                                                      	std::string ,
                                                        std::string ,
                                                        std::string& );
		
		char**															charEnvBuilder(	std::map<std::string, std::string> );
		HttpParser::httpResult											envBuilder( void );
		
		//execution
		void executor( void );
		
		int* getPipe( void );
		int	 getChild( void );
		int	 getChildStatus( void );

		// void craftHeader( std::string&  );
		// void craftBody( std::string&  );


};

#endif
