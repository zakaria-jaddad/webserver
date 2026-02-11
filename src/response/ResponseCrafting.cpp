#include "../../includes/response/Response.hpp"

std::string Response::craftHeaderStatus( std::string& header )
{
	std::string line = "HTTP/1.1 ";
	
	size_t status_pos =  header.find("status");
	
	if (status_pos == std::string::npos)
		line += " 200 OK";
	else
	{
		size_t line_end = header.find("\r\n", status_pos);
		std::string status = header.substr(status_pos + 7, line_end - (status_pos + 7));
		line.append(status);
		line.append("\n");
		header.erase(status_pos, line.size());
	}
	
	return line;
}

void Response::craftHeader( std::string& buff )
{
	std::string headers = buff;
	size_t clrf_pos = buff.find(DOUBLE_CRLF);
	if (clrf_pos != std::string::npos)
	{
    	headers = buff.substr(0, clrf_pos);
    	buff.erase(0, headers.size() + strlen(CRLF));
		this->_phase = BODY;
	}
	
	std::string line = craftHeaderStatus(headers);
	resp.append(line);
	resp.append(headers);
}

void Response::craftBody( std::string& buff )
{
	resp.append(buff);
	buff.erase();
    this->_phase = COMPLETE;
}

void Response::craftResponse( void )
{
	char buff[1024];

    int bytes = read(cgi_out_pipe[0], buff, sizeof(buff) - 1);//check errors
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

int Response::sendResponse(const char *resp) {
	if (send(client->fd, resp, strlen(resp), 0) == -1) {
		std::cerr << "failed to send" << std::endl;
		this->error = 1;
		return (1);
	}
	return (0);
}
