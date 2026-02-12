#include "../../includes/response/Response.hpp"

std::string Response::craftHeaderStatus( std::string& header )
{
	std::string line = "HTTP/1.1";
	
	size_t status_pos =  header.find("status");
	
	if (status_pos == std::string::npos)
		line += " 200 OK\r\n";
	else
	{
		size_t line_end = header.find("\r\n", status_pos);
		std::string status = header.substr(status_pos + 7, line_end - (status_pos + 7));
		line.append(status);
		// line.append("\r\n");
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
    	buff.erase(0, headers.size() + strlen(DOUBLE_CRLF));
		this->_phase = BODY;
		chunk_size = buff.length();
		std::cout << "crafted header" << std::endl;
	}
	
	std::string line = craftHeaderStatus(headers);
	head_resp.append(line);
	head_resp.append("Transfer-Encoding: chunked\r\n");
	head_resp.append(headers);
	head_resp.append(DOUBLE_CRLF);

	std::cout << "headers : " << std::endl;
	std::cout << head_resp << std::endl;
}

void Response::craftBody( std::string& buff )
{
	resp.append(buff);
	std::cout << "body : " << std::endl;
	std::cout << resp << std::endl;
	
	buff.erase();
    this->_phase = COMPLETE;
	std::cout << "crafted body" << std::endl;
}

void Response::craftResponse( void )
{
	std::cout << "--------------start of crafting response----------------" << std::endl;

	char buff[1024];

    int bytes = read(cgi_out_pipe[0], buff, sizeof(buff) - 1);//check errors
	if (bytes == -1)
	{
		error = true;
		return;
	}
    buff[bytes] = 0;
	
    std::string val = buff;
    while (val.empty() == false && this->_phase != ERROR)
    {	
		// std::cout << "responding ..." << std::endl;
        if (this->_phase == HEADERS)
			craftHeader(val);
        else if (this->_phase == BODY)
			craftBody(val);
	}

	if (!this->headerFlag && this->_phase != HEADERS)
	{
		sendResponse(head_resp.c_str());
		this->headerFlag = 1;
	}

	std::stringstream ss ;
	ss << std::hex << chunk_size;
	std::string hexStr = ss.str();
	hexStr += "\r\n";
	sendResponse(hexStr.c_str());
	std::cout << "read : " << std::endl;
	std::cout << buff << std::endl;
	
	std::cout << "--------------end of crafting response----------------" << std::endl;
}

int Response::sendResponse(const char *resp) {
	if (send(client->fd, resp, strlen(resp), 0) == -1) {
		std::cerr << "failed to send" << std::endl;
		this->error = 1;
		return (1);
	}
	return (0);
}
