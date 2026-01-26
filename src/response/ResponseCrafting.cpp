#include "../../includes/response/Response.hpp"

std::string Response::craftHeaderStatus( std::string& header )
{
	std::string line = "HTTP/1.1 ";
	
	size_t status_pos =  header.find("status");
	
	if (status_pos == std::string::npos)
		line += " 200 OK";
	else
		line.append(header.substr(status_pos + 6));
	
	header.erase(status_pos, line.size());
	
	return line;
}

void Response::craftHeader( std::string& buff )
{
	size_t clrf_pos = buff.find(DOUBLE_CRLF);
    std::string headers = buff.substr(0, clrf_pos);
    buff.erase(0, headers.size() + strlen(DOUBLE_CRLF));
    
	//parsing needed from zajaddad
	// if (HttpParser::_parseRequestHeaders(headers) != HttpParser::HTTP_RES_OK)
	// 	return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
	
	std::string line = craftHeaderStatus(headers);
    
	std::cout << "status line : \n" << line << std::endl;	
    
	std::cout << "headers : \n" << headers << std::endl;
	
	this->_phase = BODY;
}

void Response::craftBody( std::string& buff )
{
    std::cout << "BOOOOODY" << std::endl;
    this->_phase = COMPLETE;
}

void Response::craftResponse( void )
{
	cgi_pipe = cgi->getPipe();
	char buff[1024];

    int bytes = read(cgi_pipe[0], buff, sizeof(buff) - 1);//check errors
    buff[bytes] = 0;

    std::string val = buff;

    while (val.empty() == false && this->_phase != ERROR)
    {	
        if (this->_phase == HEADERS)
            craftHeader(val);
        else if (this->_phase == BODY)
            craftBody(val);
    }
}
