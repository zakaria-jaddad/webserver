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


//execution
void Cgi::executor( void )
{
	std::string cgi_path = "/usr/bin/php-cgi";

	char *argv[2] = {"/usr/bin/php-cgi", NULL};
	pid_t child = fork();
	int fds[2];
	pipe(fds);
	if (child == 0)
	{
		dup2(STDOUT_FILENO, fds[1]);
		execve(cgi_path.c_str(), argv, env);
	}
	else
	{
		int status;
		close(fds[1]);
		close(fds[0]);
		waitpid(child, &status, 0);
		delete env;
	}
}