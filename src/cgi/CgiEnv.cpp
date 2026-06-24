#include "../../includes/cgi/Cgi.hpp"

// env building

std::map<std::string, std::string>
Cgi::mapEnvBuilder(std::map<std::string, std::vector<std::string> > &header,
                   std::string script_name, std::string path_info,
                   std::string &query)
{
    // to do:
    // add a content type env var and content length
    // check if some needed env vars are needed

    // if (checkVarSanity(path_info))
    //     return std::map<std::string, std::string>();

    std::map<std::string, std::string> env;

    std::cout << "***********************CGI ENV VARS********************" << std::endl;
    std::cout << "script name : " << script_name << std::endl;
    std::cout << "path info: " << path_info << std::endl;

    env["SCRIPT_NAME"] = script_name;
    env["PATH_INFO"] = path_info;
    env["REQUEST_METHOD"] = methodToString(request->getRequestType());
    env["QUERY_STRING"] = query;

    // content length
    // if (body)
    //     env["CONTENT_LENGTH"] = content_length;
    
    // content type
    // if (content_type)
    //     env["CONTENT_TYPE"] = content_type;

    // some cgi needed? bs
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_NAME"] = "localhost";//TODO: change this value to the host requested by the client
    env["SERVER_PORT"] = conf->getPort();
    std::cout << RED << "===========PORT" << conf->getPort() << "==========" << WHITE << std::endl;
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["SERVER_SOFTWARE"] = "webserv/1.0";
    env["REMOTE_ADDR"] = "127.0.0.1"; //check get from anas

    // bs needed so php-cgi runs
    env["REDIRECT_STATUS"] = "200";

    // php specific variables
    std::string script_filename = conf->getRoot() + script_name;

    if (access(script_filename.c_str(), F_OK))
        return std::map<std::string, std::string>();

    env["SCRIPT_FILENAME"] = script_filename;
    env["DOCUMENT_ROOT"] = conf->getRoot();

    // header env variables
    headerEnvBuilder(env, header);

    return env;
}

char **Cgi::charEnvBuilder(std::map<std::string, std::string> env)
{
    char **envp = new char *[env.size() + 1];

    std::map<std::string, std::string>::iterator it = env.begin();
    int i = 0;
    for (; it != env.end(); it++) {
        std::string kv = it->first + "=" + it->second;
        // std::cout << "env crafted : " << kv << std::endl; check port somehow not getting passed properly
        envp[i] = strdup(kv.c_str());
        i++;
    }

    envp[i] = NULL;
    return envp;
}

HttpParser::httpResult Cgi::envBuilder()
{
    std::map<std::string, std::vector<std::string> > header = request->getHeaders();
    std::vector<std::string> script_name = deriveScriptName(uriPath);
    std::string query = request->getRequestUriRawParams();

    std::map<std::string, std::string> mapEnv = mapEnvBuilder(header, script_name[0], script_name[1], query);

    if (mapEnv.empty())
    {
        this->env = NULL;
        return HttpParser::HTTP_RES_NOT_IMPLEMENTED;
    }

    this->env = charEnvBuilder(mapEnv);
    return HttpParser::HTTP_RES_OK;
}
