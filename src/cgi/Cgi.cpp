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


// void craftRequestLine( std::string& buff )
// {
// 	std::string request_line = buff.substr(0, buff.find(CRLF));

// 	buff.erase(0, request_line.size() + strlen(CRLF));

//     std::cout << "request line : " << request_line << std::endl;
// }

// void craftHeader( std::string& buff )
// {
//     size_t clrf_pos = buff.find(DOUBLE_CRLF);
//     std::string headers = buff.substr(0, clrf_pos + 2);
//     buff.erase(0, headers.size() + strlen(DOUBLE_CRLF));
    
//     std::cout << "headers : " << headers << std::endl;
// }

// void craftBody( std::string& buff )
// {
//     std::cout << "BOOOOODY" << std::endl;
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
		// while (val.empty() == false)
		// {	
		// 	if (this->_phase == REQUEST_LINE)
		// 		craftRequestLine(val);
		// 	else if (this->_phase == HEADERS)
		// 		craftHeader(val);
		// 	else if (this->_phase == BODY)
		// 		craftBody(val);
		// }
		close(fds[0]);
		// delete env;
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