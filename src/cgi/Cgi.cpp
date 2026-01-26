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
	std::string cgi_path = "/usr/bin/python3";

	const char *argv[] = {"/usr/bin/python3", "/home/sanruss/Desktop/test.py", NULL};
	
  if (pipe(in_fds) == -1)
		return ;
	if (pipe(out_fds) == -1)
		return ;

//	char buff[1024];
	std::string fake_body = "username=feso&score=42&lang=c";
  size_t fake_body_len = fake_body.size();
  pid_t child = fork();

	if (child == 0)
	{
		close(out_fds[0]);
    close(in_fds[1]);

		dup2(out_fds[1], STDOUT_FILENO);
		dup2(in_fds[0], STDIN_FILENO);
    
    execve(argv[0], const_cast<char* const*>(argv), env);
	}
	else
	{
		int status;

    if (methodToString(request->getRequestType()) == "GET")
    {
      close(in_fds[0]);
      close(out_fds[1]);

      close(in_fds[1]);
    }
    else if (methodToString(request->getRequestType()) == "POST")
    {
      close(in_fds[0]);
      close(out_fds[1]);

      //write(in_fds[1], request.getBody(), request.getBodyLength());
      write(in_fds[1], fake_body.c_str(), fake_body_len);
      
      close(in_fds[1]);
    }

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
	}

}

int* Cgi::getPipe( void )
{
	return out_fds;
}

int main()
{
 	Cgi cgi;
 	cgi.executor();
}
