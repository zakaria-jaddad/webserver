#include "../../includes/cgi/Cgi.hpp"

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

std::map<std::string, std::string> createDummyMapEnv()
{
	std::map<std::string, std::string> 							env;
	
	env["SCRIPT_NAME"]		= "/test.php";
	env["PATH_INFO"]			= "";
	env["REQUEST_METHOD"]	= "GET";
	env["QUERY_STRING"]		= "";
	
	//some cgi needed? bs
	env["GATEWAY_INTERFACE"]	= "CGI/1.1";
	env["SERVER_NAME"]				= "localhost";
	env["SERVER_PORT"]				=	"8080";
	env["SERVER_PROTOCOL"]		= "HTTP/1.1";
	env["SERVER_SOFTWARE"]		= "myserver/1.0";
	env["REMOTE_ADDR"]				= "127.0.0.1";//get from anas

	//bs needed to run on terminal
	env["REDIRECT_STATUS"] = "200";
  
  //post stuff
  env["CONTENT_LENGTH"] = "29";
  env["CONTENT_TYPE"] = "application/x-www-form-urlencoded";

	//php specific variables
	env["SCRIPT_FILENAME"] = "/home/yhossni/Desktop/test.php";
	env["DOCUMENT_ROOT"]   = "/home/yhossni/Desktop";

	//header env variables
	std::map<std::string, std::vector<std::string>> dummy_header = createDummyHeader();
	headerEnvBuilder(env, dummy_header);

	return env;
}

// std::map<std::string, std::string> Cgi::mapEnvBuilder(	std::map<std::string, std::vector<std::string>>& header,
//                                                     		std::string script_name,
//                                                         std::string path_info,
//                                                         std::map<std::string, std::string>& query)
// {
// 	std::map<std::string, std::string> 							env;
	
// 	env["SCRIPT_NAME"]		= script_name;
// 	env["PATH_INFO"]			= script_name;
// 	env["REQUEST_METHOD"]	= methodToString(request->getRequestType());
// 	env["QUERY_STRING"]		= squashMapIntoString(query);
	
// 	//some cgi needed? bs
// 	env["GATEWAY_INTERFACE"]	= "CGI/1.1";
// 	env["SERVER_NAME"]				= conf->getServerName();
// 	env["SERVER_PORT"]				=	conf->getPort();
// 	env["SERVER_PROTOCOL"]		= "HTTP/1.1";
// 	env["SERVER_SOFTWARE"]		= "myserver/1.0";
// 	env["REMOTE_ADDR"]				= "127.0.0.1";//get from anas

// 	//bs needed to run on terminal
// 	env["REDIRECT_STATUS"] = "200";

// 	//php specific variables
// 	env["SCRIPT_FILENAME"] = conf->getRoot() + script_name;
// 	env["DOCUMENT_ROOT"]   = conf->getRoot();

// 	//header env variables
// 	std::map<std::string, std::vector<std::string>> dummy_header = createDummyHeader();
// 	headerEnvBuilder(env, dummy_header);

// 	return env;
// }

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
	// std::map<std::string, std::vector<std::string>> header = request->getHeaders();
    // std::map<std::string, std::string>	query = request->getRequestURIParams();

    // std::map<std::string, std::string> mapEnv = mapEnvBuilder(  header,
    //                                                             deriveScriptName(uriPath)[0],
    //                                                             deriveScriptName(uriPath)[1],
    //                                                             query);

	this->env = charEnvBuilder(createDummyMapEnv());
}
