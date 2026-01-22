#include "../../includes/cgi/Cgi.hpp"

//constructor/destructor
Cgi::Cgi()
{
	this->envBuilder();
}

Cgi::~Cgi()
{
	delete [] env;//to check
}

// std::string craftHeaderStatus( std::string& header )
// {
// 	std::string line = "HTTP/1.1 ";
	
// 	size_t status_pos =  header.find("status");
	
// 	if (status_pos == std::string::npos)
// 		line += " 200 OK";
// 	else
// 		line.append(header.substr(status_pos + 6));
	
// 	header.erase(status_pos, line.size());
	
// 	return line;
// }

// void Cgi::craftHeader( std::string& buff )
// {
// 	size_t clrf_pos = buff.find(DOUBLE_CRLF);
//     std::string headers = buff.substr(0, clrf_pos);
//     buff.erase(0, headers.size() + strlen(DOUBLE_CRLF));
    
// 	//parsing needed for zajaddad
// 	// if (HttpParser::_parseRequestHeaders(headers) != HttpParser::HTTP_RES_OK)
// 	// 	return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
	
// 	std::string line = craftHeaderStatus(headers);
    
// 	std::cout << "status line : \n" << line << std::endl;	
    
// 	std::cout << "headers : \n" << headers << std::endl;
	
// 	this->_phase = BODY;
// }

// void Cgi::craftBody( std::string& buff )
// {
//     std::cout << "BODY : \n" << buff << std::endl;
// 	buff.erase();
// 	this->_phase = COMPLETE;
// }

//execution
void Cgi::executor( void )
{
	std::string cgi_path = "/usr/bin/php-cgi";

	char *argv[2] = {"/usr/bin/php-cgi", NULL};
	if (pipe(fds) == -1)
		return ;
	char buff[1024];
	pid_t child = fork();

	if (child == 0)
	{
		close(fds[0]);
		dup2(fds[1], STDOUT_FILENO);
		execve(cgi_path.c_str(), argv, env);
	}
	else
	{
		int status;
		close(fds[1]);
		waitpid(child, &status, 0);
		// int bytes = read(fds[0], buff, sizeof(buff) - 1);
		
		// std::string val = buff;
		// while (val.empty() == false && this->_phase != ERROR)
		// {	
		// 	if (this->_phase == HEADERS)
		// 		craftHeader(val);
		// 	else if (this->_phase == BODY)
		// 		craftBody(val);
		// }
		close(fds[0]);
	}

}

int* Cgi::getPipe( void )
{
	return fds;
}

// int main()
// {
// 	Cgi cgi;
// 	cgi.executor();
// }