#include "includes/response/Response.hpp"

void Response::sendingCgiResponse()
{
    if (!this->headerFlag)
    {
        sendResponse(head_resp.c_str());
        head_resp.clear();
        if (this->_phase != HEADERS)
            this->headerFlag = 1;
    }

    if (error)
        return ;    

    if (this->headerFlag)
    {
        std::stringstream ss ;
        ss << std::hex << chunk_size;
        std::string hexStr = ss.str();
        hexStr += "\r\n";
        sendResponse(hexStr.c_str());
    }

    if (error)
        return ;
    
    if (this->_phase == BODY || this->_phase == COMPLETE)
    {
        sendResponse(resp.c_str());
        // sendResponse(CRLF);
    }

    if (error)
        return ;

    if (this->_phase == COMPLETE)
    {
        sendResponse("0\r\n\r\n");
        this->status = 1;
        this->exec_status = 0;
    }
}


HttpParser::httpResult Response::cgiExecution()
{
    // int in_pipe[2];
    // int out_pipe[2];
    
    // if (pipe(in_pipe) == -1)
    // {
    //     error = true;
    //     return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
    // }
    // if (pipe(out_pipe) == -1)
    // {
    //     error = true;
    //     return HttpParser::HTTP_RES_INTERNAL_SERVER_ERROR;
    // }
    
    // cgi_in_pipe = in_pipe;
    // cgi_out_pipe = out_pipe;
    Cgi cgi(cgi_in_pipe, cgi_out_pipe, client);
        
    cgi.executor();

    this->exec_status = 1;

	return HttpParser::HTTP_RES_OK;
}
