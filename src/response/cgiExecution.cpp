#include "../../includes/response/Response.hpp"

void Response::sendingCgiResponse()
{
    if (static_cast<size_t>(client->writeOffset) < client->outBuffer.size()) {
        ssize_t sent = sendResponse(client->outBuffer.data() + client->writeOffset,
                                    client->outBuffer.size() - client->writeOffset);

        if (sent == -2)
            return;

        if (sent > 0)
            client->writeOffset += sent;

        return;
    }

    // Everything queued has been sent.
    client->outBuffer.clear();
    client->writeOffset = 0;

    if (_phase == COMPLETE) {
        status = 1;
        exec_status = 0;
    }
}

HttpParser::httpResult Response::cgiExecution()
{
    if (this->cgi_in_pipe[0] == -1 && pipe2(this->cgi_in_pipe, O_CLOEXEC) == -1)
        return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
    if (this->cgi_out_pipe[0] == -1 && pipe2(this->cgi_out_pipe, O_CLOEXEC) == -1)
        return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;

    if (!cgi) {
        cgi = new Cgi(cgi_in_pipe, cgi_out_pipe, client);

        HttpParser::httpResult env_status = cgi->envBuilder();
        if (env_status != HttpParser::HTTP_RES_OK)
            return env_status;
    }

    cgi->executor();

    if (cgi->getChildStatus() == cgi->getChild()) {
        this->exec_status = 1;
        std::cout << GREEN << "finished executing" << WHITE << std::endl;
    }

    return HttpParser::HTTP_RES_OK;
}
