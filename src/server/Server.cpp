#include "../../includes/server/Server.hpp"
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>

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

        /*
         * socket - create an endpoint for communication
         * int socket(int domain, int type, int protocol);
         *
         * The socket has the indicated type, which specifies the communication semantics.
         *
         * - SOCK_STREAM
         *   Provides sequenced, reliable, two-way, connection-based
         *   byte streams.  An out-of-band data transmission mechanism
         *   may be supported.
         *
         * The protocol specifies a particular protocol to be used with the
         *   socket.  Normally only a single protocol exists to support a
         *   particular socket type within a given protocol family, in which
         *   case protocol can be specified as 0.
         *   serverSocket = socket(AF_INET, SOCK_STREAM, 0);
         *
         */
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);

        // Structure describing an Internet socket address.
        sockaddr_in serverAddr;

        /*
         * AF_INET is an address family which defines what type of
         * protocol is used
         * struct in_addr sin_addr; // Internet address.
         */
        serverAddr.sin_family = AF_INET;

        /* in_port_t sin_port; // Port number.   */
        serverAddr.sin_port = htons(it->getPort());

        /* unsigned char  sin_zero[8];  // Padding to match struct sockaddr size (16 bytes) */
        memset(&serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));

        /* inet_pton - convert IPv4 and IPv6 addresses from text to binary form */
        if (inet_pton(AF_INET, it->getIP().c_str(), &serverAddr.sin_addr) != 1)
            std::cerr << "error inet_pton " << it->getIP() << std::endl;

        /*
         * setsockopt — set the socket options
         *
         * The level argument specifies the protocol level at which the option resides. To set
         * options at the socket level, specify the level argument as SOL_SOCKET.
         *
         * SO_REUSEADDR  Reuse of local addresses is supported.
         *
         * optval: The configuration value for the socket option
         * */
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
            std::cerr << "error setsockopt SO_REUSEADDR " << it->getIP() << std::endl;

        if (setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == -1)
            std::cerr << "error setsockopt TCP_NODELAY " << it->getIP() << std::endl;

        if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) == -1)
            std::cerr << "error fcntl " << it->getIP() << std::endl;

        /*
         * When a socket is created with socket(2), it exists in a name space (address family) but
         * has no address assigned to it.  bind() assigns the address specified by addr to the
         * socket referred to by the file descriptor sockfd.
         */
        if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
            std::cerr << "error binding " << it->getIP() << std::endl;

        if (listen(serverSocket, SOMAXCONN) == -1)
            std::cerr << "on listen " << it->getIP() << std::endl;

        /* Add current Socket to the */
        this->vServers[serverSocket] = &(*it);
    }
}

/*
 * Add fd to epoll list of interest using EPOLL_CTL_ADD with flag meaning that we
 * are interested in read operation or write
 */
int Server::addToEpoll(int fd, uint32_t flag)
{
    epoll_event_t event;

    event.events = flag;
    event.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
        std::cerr << "epoll_ctl() Failed!" << std::endl;
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}

/*
 * Switch epoll mode (read or write)
 */
int Server::switchEpollMode(int clientSocket, uint32_t flag)
{
    epoll_event_t event;

    event.events = flag;
    event.data.fd = clientSocket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, clientSocket, &event) == -1) {
        std::cerr << "epoll_ctl() Failed!" << std::endl;
        return (1);
    }
    return (0);
}

int Server::checkServerFd(epoll_event *events, int i)
{
    // This method is just to look for fd in serverFds
    //  in case it is a serverFd it accepts and gets clientSocket and adds it to epoll & clientFds
    //  for next connection
    int clientSocket = -1;
    int opt = 1;
    HttpParser *obj;
    t_client client;
    time_t now = time(NULL);

    for (std::map<int, ServerConfig *>::iterator it = this->vServers.begin();
         it != this->vServers.end(); it++) {
        if (events[i].data.fd == it->first) {
            clientSocket = accept(it->first, NULL, NULL);
            if (clientSocket == -1) {
                std::cerr << "failed to accept" << std::endl;
                break;
            }
            if (setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == -1)
                std::cerr << "failed to set TCP_NODELAY" << std::endl;
            if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) == -1)
                std::cerr << "failed to set non-blocking" << std::endl;

            int sndbuf = 256 * 1024;
            if (setsockopt(clientSocket, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) == -1)
                std::cerr << "failed to set SO_SNDBUF" << std::endl;

            if (addToEpoll(clientSocket, EPOLLIN))
                std::cerr << "failed to add to epoll" << std::endl;
            obj = new HttpParser(it->second);
            client.fd = clientSocket;
            client.config = it->second;
            client.request = obj;
            client.response = NULL;
            client.connection_time = now;
            client.last_activity = now;
            client.writeOffset = 0;
            clients.push_back(client);
            break;
        }
    }
    return (clientSocket);
}

void Server::resetSocket(t_client *client)
{
    std::string pendingData = client->request->getPendingData();
    delete client->request;
    delete client->response;
    client->request = new HttpParser(client->config);

    // Save previous Data since connection is always open
    client->request->setPendingData(pendingData);
    client->response = NULL;
}

void Server::closeSocket(t_client *client)
{
    delete client->request;
    delete client->response;
    // Drain any unread data from the receive buffer to prevent the kernel from sending an RST
    // instead of FIN on close().
    char drain[READ_BYTES];
    int n;
    do {
        n = recv(client->fd, drain, sizeof(drain), 0);
    } while (n > 0);
    close(client->fd);
    client->request = NULL;
    client->response = NULL;
    client->config = NULL;
}

int Server::checkClientTimeout(t_client *client)
{
    time_t now = time(NULL);
    time_t idle_time = now - client->last_activity;

    if (client && !client->response && idle_time > IDLE_TIMEOUT) {
        return 1;
    }

    return 0;
}

int Server::checkClientFd(epoll_event *events, int i)
{

    // This method is to look for the event fd in clientFds
    // It then checks if the operation that is is ready for is 'read' or 'write'
    //  then reads or writes accordingly
    int clientSocket = -1;
    char req[READ_BYTES] = {0};
    int b;

    for (std::deque<t_client>::iterator it = this->clients.begin(); it != this->clients.end();) {

        if (events[i].data.fd == it->fd) {

            clientSocket = it->fd;
            if (events[i].events == EPOLLIN) {

                b = recv(clientSocket, req, sizeof(req) - 1, 0);
                if (b <= 0) {
                    closeSocket(&(*it));
                    // BUG: WTF is this for yhosnsi you were erasing the clients.begin() :|
                    clients.erase(it);
                    return 1;
                }

                it->last_activity = time(NULL);

                req[b] = 0;
                std::string rawData(req, b);

                std::cout << RED << "=================Request " << GREEN << it->fd << RED
                          << "=================" << std::endl;
                std::cout << YELLOW << rawData << WHITE << std::endl;
                it->req_res = it->request->parseHttpRequest(rawData);

                if (it->request->isComplete() || it->req_res != HttpParser::HTTP_RES_OK) {

                    switchEpollMode(clientSocket, EPOLLOUT);
                    std::cout << RED << "-----Switched epoll mode-----" << WHITE << std::endl;
                }
            }
            else if (events[i].events == EPOLLOUT) {
                // Process as many complete pipelined requests as possible in one
                // EPOLLOUT wake-up before yielding back to epoll.
                while (true) {
                    if (it->response == NULL)
                        it->response = new Response(&(*it));

                    if (it->req_res != HttpParser::HTTP_RES_OK) {
                        std::cout << "req_res : " << it->req_res << std::endl;
                        int serv_res = it->response->servePages(it->req_res, 1);
                        if (serv_res)
                            std::cout << RED << "Connection Error, closing connection ..." << WHITE
                                      << std::endl;
                        if (serv_res || it->response->isComplete()) {
                            switchEpollMode((*it).fd, EPOLL_CTL_DEL);
                            std::cout << RED << "-----Switched epoll mode-----" << WHITE
                                      << std::endl;
                            closeSocket(&(*it));
                            it = clients.erase(it);
                            std::cout << "cleaned socket" << std::endl;
                        }
                        break;
                    }

                    // start the response crafting and sending process from the checkMethod parent
                    // method
                    it->resp_res = it->response->checkMethod();

                    // cleanup after an error/timeout
                    if (it->resp_res != HttpParser::HTTP_RES_OK) {
                        std::cout << " serving error " << it->resp_res << std::endl;
                        int serv_res = it->response->servePages(it->resp_res, 1);
                        if (serv_res)
                            std::cout << RED << "Connection Error, closing connection ..." << WHITE
                                      << std::endl;
                        if (serv_res || it->response->isComplete()) {
                            switchEpollMode((*it).fd, EPOLL_CTL_DEL);
                            closeSocket(&(*it));
                            it = clients.erase(it);
                            std::cout << "cleaned socket" << std::endl;
                        }
                        break;
                    }

                    if (it->response->isComplete()) {
                        if (it->response->shouldClose()) {
                            switchEpollMode((*it).fd, EPOLL_CTL_DEL);
                            std::cout << RED << "-----Switched epoll mode-----" << WHITE
                                      << std::endl;
                            closeSocket(&(*it));
                            it = clients.erase(it);
                            std::cout << "cleaned socket on Connection: close" << std::endl;
                            break;
                        }

                        resetSocket(&(*it));

                        /* NOTE: HTTP/1.1 Pipelining ya dawla */
                        if (it->request->getPendingData().empty() == false) {

                            std::cout << GREEN << std::endl;
                            HttpParser::printRaw(it->request->getPendingData());

                            std::cout << WHITE << std::endl;
                            HttpParser::httpResult r = it->request->parseHttpRequest("");
                            it->req_res = r;

                            if (it->request->isComplete() == true ||
                                it->req_res != HttpParser::HTTP_RES_OK) {
                                std::cout << "Pipelined request ready, processing immediately"
                                          << std::endl;
                                continue;
                            }
                            switchEpollMode(clientSocket, EPOLLIN);
                            std::cout << "Pipelined request needs more data, back to reading"
                                      << std::endl;
                            break;
                        }
                        switchEpollMode(clientSocket, EPOLLIN);
                        std::cout << "back to reading" << std::endl;
                    }
                    break;
                }
                break;
            }
            break;
        }
        it++;
    }
    return (clientSocket);
}

/*
 *
 * */
int Server::startServer()
{
    int clientSocket;
    int event_count;

    setupServerFds();

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        std::cerr << "epoll_create1() Failed!" << std::endl;
        return 1;
    }

    epoll_event_t events[MAX_EVENTS];

    for (std::map<int, ServerConfig *>::iterator it = this->vServers.begin();
         it != this->vServers.end(); it++) {
        if (addToEpoll(it->first, EPOLLIN)) {
            return EXIT_FAILURE;
        }
    }

    while (1) {

        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 100);

        if (event_count == -1) {
            std::cerr << "epoll_wait Failed!" << std::endl;
            return (1);
        }
        if (event_count == 0) {
            for (std::deque<t_client>::iterator it = this->clients.begin();
                 it != this->clients.end();) {
                int isTimeout = checkClientTimeout(&(*it));
                if (isTimeout) {
                    std::cout << "client fd=" << (*it).fd << " timed out " << std::endl;
                    std::cout << "done sending timeout resetting socket..." << std::endl;
                    switchEpollMode(it->fd, EPOLL_CTL_DEL);
                    std::cout << RED << "-----Switched epoll mode-----" << WHITE << std::endl;
                    closeSocket(&(*it));
                    it = clients.erase(it);
                    continue;
                }
                it++;
            }
        }

        for (int i = 0; i < event_count; i++) {

            clientSocket = checkServerFd(events, i);
            if (clientSocket != -1) {
                continue;
            }
            clientSocket = checkClientFd(events, i);
            if (clientSocket == -1) {
                std::cerr << "Error: ClientSocket not found!" << std::endl;
            }
        }
    }
    return (0);
}
