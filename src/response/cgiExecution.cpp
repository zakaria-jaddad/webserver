#include "../../includes/response/Response.hpp"

void Response::sendingCgiResponse()
{
    if (!this->headerFlag) {
        sendResponse(head_resp.c_str(), head_resp.size());
        head_resp.clear();
        if (this->_phase != HEADERS)
            this->headerFlag = 1;
    }

    if (error)
        return;

    if (this->headerFlag) {
        std::stringstream ss;
        ss << std::hex << chunk_size;
        std::string hexStr = ss.str();
        hexStr += "\r\n";
        sendResponse(hexStr.c_str(), hexStr.size());
        chunk_size = 0;
    }

    if (error)
        return;

    if (this->_phase == BODY) {
        sendResponse(resp.c_str(), resp.size());
    }

    if (error)
        return;

    if (this->_phase == COMPLETE) {
        sendResponse("0\r\n\r\n", 5);
        this->status = 1;
        this->exec_status = 0;
    }
}

HttpParser::httpResult Response::cgiExecution()
{
    if (!cgi) {
        cgi = new Cgi(cgi_in_pipe, cgi_out_pipe, client);

        HttpParser::httpResult env_status = cgi->envBuilder();
        if (env_status != HttpParser::HTTP_RES_OK)
            return env_status;
    }

    cgi->executor();

    if (cgi->getChildStatus() == cgi->getChild()) {
        this->exec_status = 1;
        std::cout << "finished executing" << std::endl;
    }

    return HttpParser::HTTP_RES_OK;
}
