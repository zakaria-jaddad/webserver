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
std::string	Cgi::joinHeaderValues(std::vector<std::string>& field_value)
{
	std::string	env_val;
	
	for (int i = 0; i < field_value.size(); i++)
	{
		env_val += field_value[i] + ", ";
	}

	return env_val;
}

std::string Cgi::normalizeHeaderName(const std::string& name)
{
	std::string header = "HTTP_" + name;

	for (int i = 0; i < header.length(); i++)
	{
		char c = header[i];

		if (c == '-')
			header[i] = '_';
		if (std::isalpha(c))
			header[i] = std::toupper(c);
	}

	return header;
}

void Cgi::headerEnvBuilder( std::map<std::string, std::string>& env, std::map<std::string, std::vector<std::string>>& header )
{
	std::map<std::string, std::vector<std::string>>::iterator it = header.begin();

	for (; it != header.end(); it++)
	{
		std::string	field_value = joinHeaderValues(it->second);
		std::string	field_name = normalizeHeaderName(it->first);
		env[field_name] = field_value;
	}
}

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

	env["SCRIPT_NAME"]		= "/test.php";
	env["REQUEST_METHOD"]	= "GET";
	env["QUERY_STRING"]		= "";
	
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
	env["SCRIPT_FILENAME"] = "/home/yhossni/Desktop/test.php";
	env["DOCUMENT_ROOT"]   = "/home/yhossni/Desktop";

	//so far, allegedly, missing header variables

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
	Cgi cgi;
	cgi.executor();

	// std::string key = "X-forwarded-For";
	// std::vector<std::string> val = {"gz", "sdf", "Qwer"};

	// std::cout << normalizeHeaderName(key) << "\n" << joinHeaderValues(val) << std::endl;
}