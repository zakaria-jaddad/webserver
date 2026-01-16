#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "ResolvedConfig.hpp"
#include "../server/Server.hpp"

class Response{

	ResolvedConfig conf;

public:
	Response( const t_client* );

};

#endif