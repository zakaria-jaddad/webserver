#include "../../includes/response/Response.hpp"

void Response::craftRequestLine( std::string& buff )
{
	std::string request_line = buff.substr(0, buff.find(CRLF));

	buff.erase(0, request_line.size() + strlen(CRLF));

    std::cout << "request line : " << request_line << std::endl;

    this->_phase = HEADERS;
}

void Response::craftHeader( std::string& buff )
{
    size_t clrf_pos = buff.find(DOUBLE_CRLF);
    std::string headers = buff.substr(0, clrf_pos + 2);
    buff.erase(0, headers.size() + strlen(DOUBLE_CRLF));
    
    std::cout << "headers : " << headers << std::endl;
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

    int bytes = read(cgi_pipe[0], buff, sizeof(buff) - 1);
    std::string val = buff;
	
    while (val.empty() == false)
	{
		if (this->_phase == REQUEST_LINE)
			craftRequestLine(val);
		else if (this->_phase == HEADERS)
			craftHeader(val);
		else if (this->_phase == BODY)
			craftBody(val);
	}
}