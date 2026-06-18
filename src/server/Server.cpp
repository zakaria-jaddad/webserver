#include "../../includes/server/Server.hpp"
#include <cstdlib>
#include <fstream>
#include <map>
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
        perror("ctl failed");
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
    time_t now = time(NULL);

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
            client.connection_time = now;
            client.last_activity = now;
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

int Server::checkClientTimeout(t_client *client)
{
    time_t now = time(NULL);
    time_t idle_time = now - client->last_activity;

    // std::cout << "[" << idle_time << "]" << "idle value for fd=" << client->fd << std::endl;
    // std::cout << "[" << now << "]" << "checking timeout..." << std::endl;

    if (client->request && !client->request->isComplete() &&
        idle_time > REQUEST_TIMEOUT) // check if i should just not implement the request timeout
    {
        std::cout << "Request status: " << client->request->isComplete() << " and timed out\n";
        return 2;
    }

    if (client->request && client->request->isComplete() && !client->response &&
        idle_time > IDLE_TIMEOUT) {
        std::cout << "Request status: " << client->request->isComplete() << " and idle timeout\n";
        return 1;
    }

    return 0;
}

int Server::checkForClientFd(epoll_event *events, int i)
{

    // This method is to look for the event fd in clientFds
    // It then checks if the operation that is is ready for is 'read' or 'write'
    //  then reads or writes accordingly
    int clientSocket = -1;
    char req[READ_BYTES] = {0};
    int b;
    int c = 0;

    for (std::deque<t_client>::iterator it = this->clients.begin(); it != this->clients.end();
         c++) {
        // std::cout << "looping thru clients" << std::endl;

        // Request timeout/client idling too long
        int isTimeout = checkClientTimeout(&(*it));
        if (isTimeout) {
            // for request timeout
            if (isTimeout == 2) {
                it->response = new Response(&(*it));
                it->response->servePages(408);
                std::cout << "request timed out!!\n";
            }

            // general cleanup (either idle or request timeout)
            std::cout << "client fd=" << (*it).fd << " timed out " << std::endl;
            switchEpollMode((*it).fd, EPOLL_CTL_DEL);
            closeSocket(&(*it));
            it = clients.erase(clients.begin() + c);
            continue;
        }

        if (events[i].data.fd == it->fd) {
            clientSocket = it->fd;
            if (events[i].events == EPOLLIN) {
                b = recv(clientSocket, req, sizeof(req) - 1, 0);
                if (b == -1 || b == 0) {
                    closeSocket(&(*it));
                    clients.erase(clients.begin() + c);
                    return 1;
                    // TODO:check if i must remove from epoll list of interest
                }
                req[b] = 0;
                it->last_activity = time(NULL);

                std::string rawData(req, b);

                HttpParser::httpResult r = it->request->parseHttpRequest(rawData);
                std::cout << "old=" << it->req_res << " new=" << r << std::endl;

                it->req_res = r;

                if (it->request->isComplete() || it->req_res != HttpParser::HTTP_RES_OK)
                    switchEpollMode(clientSocket, EPOLLOUT);
            }
            else if (events[i].events == EPOLLOUT) {
                // we can write to socket
                if (it->response == NULL)
                    it->response = new Response(&(*it));

                if (it->req_res != HttpParser::HTTP_RES_OK) {
                    std::cout << "req_res : " << it->req_res << std::endl;
                    it->response->servePages(it->req_res);
                    if (it->response->isComplete()) {
                        switchEpollMode((*it).fd, EPOLL_CTL_DEL);
                        closeSocket(&(*it));
                        it = clients.erase(clients.begin() + c);
                        std::cout << "cleaned socket" << std::endl;
                    }
                    break;
                }

                // start the response crafting and sending process from the checkMethod parent
                // method
                it->resp_res = it->response->checkMethod();

                // cleanup after an error/timeout
                if (it->resp_res != HttpParser::HTTP_RES_OK) {
                    // std::cout << "cgi timed out" << std::endl;
                    switchEpollMode((*it).fd, EPOLL_CTL_DEL);
                    closeSocket(&(*it));
                    it = clients.erase(clients.begin() + c);
                    std::cout << "cleaned socket" << std::endl;
                    break;
                }

                if (it->response->isComplete()) {
                    resetSocket(&(*it));
                    switchEpollMode(clientSocket, EPOLLIN);
                    std::cout << "back to reading" << std::endl;
                }
                // check if connection close or keep alive to know
                // whether to close or in case of keep alive clear data buff falgs etc..
            }
            break;
        }
        it++;
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

        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 0);

        if (event_count == -1) {
            std::cerr << "epoll_wait Failed!" << std::endl;
            return (1);
        }
        if (event_count == 0) {
            int c = 0;
            for (std::deque<t_client>::iterator it = this->clients.begin();
                 it != this->clients.end(); c++) {
                // std::cout << "looping thru clients" << std::endl;

                // check possibility to keep this and remove the above timeout check
                int isTimeout = checkClientTimeout(&(*it));
                if (isTimeout) {

                    // for request timeout
                    if (isTimeout == 2) {
                        it->response = new Response(&(*it));
                        it->response->servePages(408);
                        std::cout << "request timed out!!\n";
                    }

                    std::cout << "client fd=" << (*it).fd << " timed out " << std::endl;
                    std::cout << "done sending timeout resetting socket..." << std::endl;
                    switchEpollMode(it->fd, EPOLL_CTL_DEL);
                    closeSocket(&(*it));
                    it = clients.erase(clients.begin() + c);
                    continue;
                }
                it++;
            }
        }

        for (int i = 0; i < event_count; i++) {

            clientSocket = checkForServerFd(events, i);
            if (clientSocket != -1) // found fd in serverFds
                continue;
            clientSocket = checkForClientFd(events, i);
            if (clientSocket == -1)
                std::cerr << "error clienSocket Not Found!" << std::endl;
        }
    }
    return (0);
}
