#include "../../includes/cgi/Cgi.hpp"

//constructor/destructor
Cgi::Cgi( int* in, int* out, t_client* client )
: in_fds(in), out_fds(out), request(client->request)
{
	std::cout << "--------------start of cgi constructor----------------" << std::endl;

	
	uriPath = request->getRequestURIPath();
	std::cout << "uri : " << uriPath << std::endl;
	
	conf = new ResolvedConfig(uriPath, client->config);

	scriptPath = conf->getRoot() + deriveScriptName(uriPath)[0];
	std::cout << "script path : " << scriptPath << std::endl;

	cgiPath = conf->getCgi().second;
	
	
	this->envBuilder();

	std::cout << "--------------end of cgi constructor----------------" << std::endl;
}

Cgi::~Cgi()
{
	delete conf;
	delete [] env;//to check
}

//execution
void Cgi::executor( void )
{
	const char *argv[] = {cgiPath.c_str(), scriptPath.c_str(), NULL};
	// std::cout << "cgi path : " << std::endl;
	// std::cout << cgiPath << std::endl;
	// std::cout << "script path : " << std::endl;
	// std::cout << scriptPath << std::endl;
	// int in_fds[2];
	// int out_fds[2];

	if (pipe(in_fds) == -1)
		return;
	if (pipe(out_fds) == -1)
		return;

  pid_t child = fork();

	if (child == 0)
	{
		close(out_fds[0]);
    close(in_fds[1]);

		dup2(out_fds[1], STDOUT_FILENO);
		dup2(in_fds[0], STDIN_FILENO);
    
    execve(cgiPath.c_str(), const_cast<char* const*>(argv), env);
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
      // write(in_fds[1], fake_body.c_str(), fake_body_len);
      
      close(in_fds[1]);
    }

	  waitpid(child, &status, 0);
		std::cout << "finished executing" << std::endl;
	}

}

int* Cgi::getPipe( void )
{
	return out_fds;
}

// int main()
// {
//  	Cgi cgi;
//  	cgi.executor();
// }
