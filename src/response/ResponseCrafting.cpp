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
		
		head_resp.append(headers);

		// std::cout << "+++++++ header to send next ++++++++" << std::endl;
		// std::cout << head_resp << std::endl;

		head_resp.append(CRLF);		
		this->_phase = BODY;
	}
	else
	{
		head_resp.append(headers);
		
		// std::cout << "+++++++ header to send next ++++++++" << std::endl;
		// std::cout << head_resp << std::endl;
		
		buff.erase();
	}
	
	if (this->_phase == BODY)
	{
		std::string line = craftHeaderStatus(headers);
		head_resp.insert(0, line);
		head_resp.append("Transfer-Encoding: chunked\r\n");
		head_resp.append(DOUBLE_CRLF);

		// std::cout << "+++++++ header to send next ++++++++" << std::endl;
		// std::cout << head_resp << std::endl;
	}
	
	// std::cout << "crafted header" << std::endl;
}

void Response::craftBody( std::string& buff )
{
	resp.append(buff);
	buff.erase();

	// std::cout << "crafted body : " << std::endl;
	// std::cout << resp << std::endl;
}

HttpParser::httpResult Response::craftResponse( void )
{
	std::cout << "--------------start of crafting response----------------" << std::endl;

	char buff[10];
	std::string val;

	while (this->_phase == HEADERS && !this->error)
	{
		ssize_t bytes = read(cgi_out_pipe[0], buff, sizeof(buff) - 1);

		if (bytes == 0)
		{
			std::cout << "nothing else to read" << std::endl;
			this->_phase = COMPLETE;
			break;
		}

		if (bytes == -1)
		{
			perror("read failed");
			error = true;
			return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
		}

		buff[bytes] = 0;

		val = buff;

		craftHeader(val);
	}

	if (this->_phase == BODY)
	{
		if (val.empty())
		{
			ssize_t bytes = read(cgi_out_pipe[0], buff, sizeof(buff) - 1);

			if (bytes == 0)
			{
				this->_phase = COMPLETE;
				return HttpParser::HTTP_RES_OK;
			}

			if (bytes == -1)
			{
				perror("read failed");
				error = true;
				return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
			}

			buff[bytes] = 0;

			val = buff;
		}

		chunk_size = val.length();

		craftBody(val);
	}

	std::cout << "--------------end of crafting response----------------" << std::endl;

	return HttpParser::HTTP_RES_OK;
}

int Response::sendResponse(const char *resp) {
	if (send(client->fd, resp, strlen(resp), 0) == -1) { // check later this function's errors should be taken into consideration
		std::cerr << "failed to send" << std::endl;
		this->error = 1;
		return (1);
	}
	return (0);
}
