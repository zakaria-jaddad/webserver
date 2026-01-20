#include "../../includes/cgi/Cgi.hpp"
#include <unistd.h>
#include <sys/wait.h> 

// void cgiHandler( std::map<std::string, std::string>& cgi, HttpParser& req )
// {
//     std::map<std::string, std::string>::iterator it_cgi = cgi.begin();



// }

char** cgiEnvBuilder()
{
	std::map<std::string, std::string> env;
	
	env["SCRIPT_NAME"] = "/test.php";
	env["REQUEST_METHOD"] = "GET";
	env["QUERY_STRING"] = "";
	
	//some cgi needed? bs
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SERVER_NAME"] = "localhost";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["SERVER_SOFTWARE"]   = "myserver/1.0";
	env["SERVER_PORT"]       = "8080";
	env["REMOTE_ADDR"] = "127.0.0.1";

	//bs needed to run on terminal
	env["REDIRECT_STATUS"] = "200";

	//php specific variables
	env["SCRIPT_FILENAME"] = "/home/yhossni/Desktop/test.php";
	env["DOCUMENT_ROOT"]   = "/home/yhossni/Desktop";

	//so far, allegedly, missing header variables


	char **envp = new char *[env.size() + 1];

	std::map<std::string, std::string>::iterator it = env.begin();
	int i = 0;
	for (; it != env.end(); it++)
	{
		std::string kv = it->first + "=" + it->second;
		envp[i] = strdup(kv.c_str());
		i++;
	}

	return envp;
}

void cgiExecutor()
{
	std::string cgi_path = "/usr/bin/php-cgi";
	char** env = cgiEnvBuilder();


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
	}

}

int main()
{
	cgiExecutor();
}