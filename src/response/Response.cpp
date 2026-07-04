#include "../../includes/response/Response.hpp"

Response::Response(t_client *dep)
    : conf(dep->request->getRequestUriPath(), dep->config), _phase(HEADERS)
{
    this->wait = 0;
    this->client = dep;
    this->fileFD = -3;
    this->headerFlag = 0;
    this->status = 0;
    this->exec_status = 0;
    this->error = 0;
    this->timed_out = 0;
    this->start_time = time(NULL);
    this->cgi = NULL;
    this->closeConnection = true;
    if (dep->request != NULL && dep->request->isComplete())
        this->closeConnection = !dep->request->keepAlive();

    this->cgi_in_pipe[0] = -1;
    this->cgi_in_pipe[1] = -1;
    this->cgi_out_pipe[0] = -1;
    this->cgi_out_pipe[1] = -1;

    this->_pageFd = -1;
}

Response::~Response(void)
{
    if (cgi)
        delete cgi;

    if (this->fileFD >= 0)
        close(this->fileFD);

    for (int i = 0; i < 2; ++i) {
        if (this->cgi_in_pipe[i] >= 0)
            close(this->cgi_in_pipe[i]);
        if (this->cgi_out_pipe[i] >= 0)
            close(this->cgi_out_pipe[i]);
    }
}

HttpParser::httpResult Response::routeMethod(HttpParser::requestType req_method)
{
    const std::string &uri = client->request->getRequestUriPath();

    if (isCgi(uri, conf.getCgi())) {
        // std::cout << time(NULL) - start_time << std::endl;
        if ((time(NULL) - start_time) > CGI_TIMEOUT) {
            this->timed_out = 1;
            int childpid = cgi->getChild();

            std::cout << "child to kill : " << childpid << std::endl;

            kill(childpid, SIGKILL);
            waitpid(childpid, NULL, 0);
            return HttpParser::HTTP_RES_GATEWAY_TIMEOUT;
        }

        if (!this->exec_status) {
            HttpParser::httpResult res = cgiExecution();

            if (res != HttpParser::HTTP_RES_OK) {
                return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
            }
            return HttpParser::HTTP_RES_OK;
        }

        std::cout << "cgi executed!" << std::endl;
        if (this->_phase != COMPLETE) {
            HttpParser::httpResult cr = craftResponse();
            if (cr != HttpParser::HTTP_RES_OK) {
                return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
            }
        }

        sendingCgiResponse();

        if (error) {
            return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
        }

        std::cout << "Done sending the response" << std::endl;
        resp.clear();
    }

    else {
        switch (req_method) {
        case HttpParser::DELETE:
            return (httpDelete());
        case HttpParser::GET:
            return (httpGet());
        case HttpParser::POST:
            return (httpPost());
        default:
            return HttpParser::HTTP_RES_METHOD_NOT_ALLOWED;
        }
    }

    return HttpParser::HTTP_RES_OK;
}

HttpParser::httpResult Response::checkMethod(void)
{
    // check if its needed
    // if (this->timed_out)
    // 	return HttpParser::HTTP_RES_GATEWAY_TIMEOUT;//placeholder

    std::pair<int, std::string> redir = conf.getRedir();
    if (!redir.second.empty()) {
        std::cout << YELLOW << "Redirecting : " << redir.first << " ==== " << redir.second << WHITE
                  << std::endl;
        if (servePages(redir.first, redir.second, 0)) {
            std::cout << RED << "Error Redirecting" << WHITE << std::endl;
            return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
        }
        return HttpParser::HTTP_RES_OK;
    }
    HttpParser::requestType req_method = client->request->getRequestType();
    std::vector<std::string> allowed_methods = conf.getAcceptedMethods();

    if (!allowed_methods.empty() &&
        std::find(allowed_methods.begin(), allowed_methods.end(), methodToString(req_method)) ==
            allowed_methods.end()) {
        return HttpParser::HTTP_RES_METHOD_NOT_ALLOWED;
    }

    return routeMethod(req_method);
}
