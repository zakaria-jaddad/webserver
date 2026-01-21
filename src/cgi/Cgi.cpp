#include "../../includes/cgi/Cgi.hpp"
#include <unistd.h>
#include <sys/wait.h> 


//constructor/destructor
Cgi::Cgi()
{
	this->envBuilder();
}

Cgi::~Cgi()
{
	delete [] env;//to check
}


//env building
std::map<std::string, std::vector<std::string>> createDummyHeader()
{
	std::map<std::string, std::vector<std::string>> header;

	// Single-value header
	header["Host"].push_back("example.com");

	// Multi-value header (repeated field-name)
	header["Accept"].push_back("text/html");
	header["Accept"].push_back("application/json");

	// Another multi-value header
	header["Cache-Control"].push_back("no-cache");
	header["Cache-Control"].push_back("no-store");

	// Header with comma already inside a value
	header["X-Forwarded-For"].push_back("203.0.113.1");
	header["X-Forwarded-For"].push_back("198.51.100.42");

	// Header you may want to skip in CGI
	header["Content-Type"].push_back("text/plain");

	return header;
}

std::map<std::string, std::string> Cgi::mapEnvBuilder()
{
	std::map<std::string, std::vector<std::string>> header = request->getHeaders();
	std::map<std::string, std::string> 							env;
	
	std::string script_name = deriveScriptName(uriPath)[0];//i assume unsafe
	std::string path_info = deriveScriptName(uriPath)[1];//i assume unsafe
	std::map<std::string, std::string>	query = request->getRequestURIParams();

	env["SCRIPT_NAME"]		= script_name;
	env["PATH_INFO"]			= script_name;
	env["REQUEST_METHOD"]	= methodToString(request->getRequestType());
	env["QUERY_STRING"]		= squashMapIntoString(query);
	
	//some cgi needed? bs
	env["GATEWAY_INTERFACE"]	= "CGI/1.1";
	env["SERVER_NAME"]				= conf->getServerName();
	env["SERVER_PORT"]				=	conf->getPort();
	env["SERVER_PROTOCOL"]		= "HTTP/1.1";
	env["SERVER_SOFTWARE"]		= "myserver/1.0";
	env["REMOTE_ADDR"]				= "127.0.0.1";//get from anas

	//bs needed to run on terminal
	env["REDIRECT_STATUS"] = "200";

	//php specific variables
	env["SCRIPT_FILENAME"] = conf->getRoot() + script_name;
	env["DOCUMENT_ROOT"]   = conf->getRoot();

	//header env variables
	std::map<std::string, std::vector<std::string>> dummy_header = createDummyHeader();
	headerEnvBuilder(env, dummy_header);

	return env;
}

char** Cgi::charEnvBuilder(std::map<std::string, std::string> env)
{
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

void Cgi::envBuilder()
{
	this->env = charEnvBuilder(mapEnvBuilder());
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

int main()
{
	// Cgi cgi;
	// cgi.executor();

	// std::string path = "/asdf/des/tst/tes";
	// deriveScriptName(path);
	// std::string key = "X-forwarded-For";
	// std::vector<std::string> val = {"gz", "sdf", "Qwer"};

	// std::cout << normalizeHeaderName(key) << "\n" << joinHeaderValues(val) << std::endl;
}