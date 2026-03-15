#include "../../includes/server/Server.hpp"
#include <cstdlib>
#include <sstream>
#include <sys/epoll.h>

Server::Server(std::string &path)
{
    this->conf.tokenize(path);
    this->conf.loadServers();
}

void Server::setupServerFds()
{
    int serverSocket;
    int opt = 1;

    for (std::vector<ServerConfig>::iterator it = this->conf.servers.begin();
         it != this->conf.servers.end(); it++) {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in serverAddr;

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(it->getPort());
        if (inet_pton(AF_INET, it->getIP().c_str(), &serverAddr.sin_addr) != 1)
            std::cerr << "error inet_pton " << it->getIP() << std::endl;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
            std::cerr << "error setsockopt " << it->getIP() << std::endl;
        if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
            std::cerr << "error binding " << it->getIP() << std::endl;
        if (listen(serverSocket, 10) == -1) // TODO: update later to get it from conf
            std::cerr << "on listen " << it->getIP() << std::endl;
        // std::cout << "added socker fd : " << serverSocket << std::endl;
        this->vServers[serverSocket] = &(*it);
    }
}

int Server::addToEpoll(int fd, uint32_t flag)
{
    // This method is to add fd to epoll list of interest
    //  with flag meaning that we are interested in read operation or write ...
    epoll_event_t event;

    event.events = flag;
    event.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
        std::cerr << "epoll_ctl Failed!" << std::endl;
        return (1);
    }
    return (0);
}

int Server::switchEpollMode(int clientSocket, uint32_t flag)
{
    // This method is to switch epoll mode (read or write)
    epoll_event_t event;

    event.events = flag;
    event.data.fd = clientSocket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, clientSocket, &event) == -1) {
        std::cerr << "epoll_ctl Failed!" << std::endl;
        return (1);
    }
    return (0);
}

int Server::checkForServerFd(epoll_event *events, int i)
{
    // This method is just to look for fd in serverFds
    //  in case it is a serverFd it accepts and gets clientSocket and adds it to epoll & clientFds
    //  for next connection
    int clientSocket = -1;
    HttpParser *obj;
    t_client client;

    for (std::map<int, ServerConfig *>::iterator it = this->vServers.begin();
         it != this->vServers.end(); it++) {
        if (events[i].data.fd == it->first) {
            clientSocket = accept(it->first, NULL, NULL);
            if (clientSocket == -1)
                std::cerr << "failed to accept" << std::endl;
            if (addToEpoll(clientSocket, EPOLLIN))
                std::cerr << "failed to add to epoll" << std::endl;
            obj = new HttpParser(it->second);
            client.fd = clientSocket;
            client.config = it->second;
            client.request = obj;
            client.response = NULL;
            clients.push_back(client);
            break;
        }
    }
    return (clientSocket);
}

void Server::resetSocket(t_client *client)
{
    delete client->request;
    delete client->response;
    client->request = new HttpParser(client->config);
    client->response = NULL;
}

void Server::closeSocket(t_client *client)
{
    delete client->request;
    delete client->response;
    // delete client->config;
    close(client->fd);
    client->request = NULL;
    client->response = NULL;
    client->config = NULL;
}

int Server::checkForClientFd(epoll_event *events, int i)
{
    // This method is to look for the event fd in clientFds
    // It then checks if the operation that is is ready for is 'read' or 'write'
    //  then reads or writes accordingly

    int clientSocket = -1;
    char req[1024] = {0};
    int b;
    int c = 0;

    for (std::vector<t_client>::iterator it = this->clients.begin(); it != this->clients.end();
         it++, c++) {
        if (events[i].data.fd == it->fd) {
            clientSocket = it->fd;
            // std::cout << "already in map clientSocket = " << clientSocket << std::endl;
            if (events[i].events == EPOLLIN) { // we can read from socket
                b = recv(clientSocket, req, sizeof(req) - 1, 0);
                if (b == -1 || b == 0) {
                    closeSocket(&(*it));
                    clients.erase(clients.begin() + c);
                    break;
                    // TODO:check if i must remove from epoll list of interest
                }
                req[b] = 0;
                HttpParser::httpResult res = it->request->parseHttpRequest(req);
                it->request->setRes(res);
                if (res != HttpParser::HTTP_RES_OK) {
                    std::cout << RED << std::endl;
                    it->request->printHttpResult(res);
                    std::cout << WHITE << std::endl;
                    exit(0);
                }

                // debugging parsed request
                // std::cout << "=======================" << std::endl;
                // std::map<std::string, std::vector<std::string> > foo = it->request->getHeaders();
                //
                // for (std::map<std::string, std::vector<std::string> >::iterator itMap =
                // foo.begin();
                //      itMap != foo.end(); ++itMap) {
                //     std::string key = itMap->first;
                //     std::vector<std::string> &values = itMap->second;
                //
                //     std::cout << "Header: " << key << std::endl;
                //
                //     for (std::vector<std::string>::iterator itVec = values.begin();
                //          itVec != values.end(); ++itVec) {
                //         std::cout << "  Value: " << *itVec << std::endl;
                //     }
                // }
                // std::cout << "=======================" << std::endl;

                if (it->request->isComplete())
                    switchEpollMode(clientSocket, EPOLLOUT);
            }
            else if (events[i].events == EPOLLOUT) { // we can write to socket
                if (it->response == NULL)
                    it->response = new Response(&(*it));
                it->response->checkMethod();
                if (it->response->isError()) {
                    closeSocket(&(*it));
                    clients.erase(clients.begin() + c);
                    break;
                    // hh
                }
                if (it->response->isComplete()) {
                    resetSocket(&(*it));
                    switchEpollMode(clientSocket, EPOLLIN);
                    std::cout << "back to reading" << std::endl;
                }
                // check if connection close or keep alive to know
                // whether to close or in case of keep alive clear data buff flags etc..
            }
            break;
        }
    }
    return (clientSocket);
}

int Server::startServer()
{

    setupServerFds();
    int clientSocket;
    int event_count;

    epoll_fd = epoll_create1(0);
    epoll_event_t events[MAX_EVENTS];

    for (std::map<int, ServerConfig *>::iterator it = this->vServers.begin();
         it != this->vServers.end(); it++) {
        if (addToEpoll(it->first, EPOLLIN))
            return (1);
    }

    while (1) {

        /* Zakaria Add This Section */

        // bool backgroundWork = false;
        // for (size_t i = 0; i < this->clients.size(); i++) {
        //
        //     // if client is found and body is processing
        //     if (this->clients[i].request->isProcessingBody() == true) {
        //         backgroundWork = true;
        //     }
        //
        //     /* NOTE: Since we read from the file no data need to be passed to the parser */
        //     this->clients[i].request->parseHttpRequest("");
        //
        //     if (this->clients[i].request->isComplete() == true) {
        //         switchEpollMode(this->clients[i].fd, EPOLLOUT);
        //     }
        // }
        //
        // /* NOTE: If we have clients processing local files, we CANNOT let epoll block the server.
        // We
        //  * set timeout to 0 (non-blocking). Otherwise, we block for 30s to save CPU. */
        // int timeout = backgroundWork ? 0 : 30000;

        /* End of Zakaria Add This Section */

        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000);
        if (event_count == -1) {
            std::cerr << "epoll_wait Failed!" << std::endl;
            return (1);
        }
        // std::cout << "event_count = " << event_count << std::endl;
        for (int i = 0; i < event_count; i++) {
            clientSocket = checkForServerFd(events, i);
            if (clientSocket != -1) // found fd in serverFds
                continue;
            clientSocket = checkForClientFd(events, i);
            if (clientSocket == -1)
                std::cerr << "error clientSocket Not Found!" << std::endl;
        }
    }
    return (0);
}
