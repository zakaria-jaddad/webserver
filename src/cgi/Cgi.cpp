#include "../../includes/cgi/Cgi.hpp"

// constructor/destructor
Cgi::Cgi(int *in, int *out, t_client *client) 
:  request(client->request), in_fds(in), out_fds(out), child(-10), child_status(-5)
{
    std::cout << "--------------start of cgi constructor----------------" << std::endl;

    uriPath = request->getRequestUriPath();
    std::cout << "uri : " << uriPath << std::endl;

    conf = new ResolvedConfig(uriPath, client->config);

    scriptPath = conf->getRoot() + deriveScriptName(uriPath)[0];
    std::cout << "script path : " << scriptPath << std::endl;

    cgiPath = conf->getCgi().second;

    std::cout << "--------------end of cgi constructor----------------" << std::endl;
}

Cgi::~Cgi()
{
    delete conf;
    delete[] env; // to check
}

// execution
void Cgi::executor(void)
{
    if (child == -10)
    {
        const char *argv[] = {cgiPath.c_str(), scriptPath.c_str(), NULL};
    
        child = fork();
    
        if (child == 0) {
            close(out_fds[0]);
            close(in_fds[1]);
    
            // std::cout << "params passed to execve: " << std::endl;
            // std::cout << "cgi path : " << cgiPath << std::endl;
            // for (int i = 0; argv[i]; i++)
            //     std::cout << "argv[" << i << "] : " << argv[i] << std::endl;
            
            // for (int i = 0; env[i]; i++)
            //     std::cout << "env[" << i << "] : " << env[i] << std::endl;

            dup2(out_fds[1], STDOUT_FILENO);
            dup2(in_fds[0], STDIN_FILENO);
    
            execve(cgiPath.c_str(), const_cast<char *const *>(argv), env);
            perror("execve");
        }
        else {
    
            std::cout << "child created : " << child << std::endl;
            if (methodToString(request->getRequestType()) == "GET") {
                close(in_fds[0]);
                close(out_fds[1]);
    
                close(in_fds[1]);
            }
            else if (methodToString(request->getRequestType()) == "POST") {
                close(in_fds[0]);
                close(out_fds[1]);
    
                // write(in_fds[1], request.getBody(), request.getBodyLength());
                //  write(in_fds[1], fake_body.c_str(), fake_body_len);
    
                close(in_fds[1]);
            }
    
        }
    }

    int status;
    // std::cout << "waiting for child" << std::endl;
    child_status = waitpid(child, &status, WNOHANG);
}

int *Cgi::getPipe(void) { return out_fds; }
int Cgi::getChild(void) { return child; }
int Cgi::getChildStatus(void) { return child_status; }

// int main()
// {
//  	Cgi cgi;
//  	cgi.executor();
// }
