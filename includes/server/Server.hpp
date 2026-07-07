#ifndef SERVER_HPP
#define SERVER_HPP

#include "../config/Config.hpp"
#include "../request/Request.hpp"
#include "../response/Response.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <list>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

typedef struct epoll_event epoll_event_t;
#define MAX_EVENTS 1024
#define REQUEST_TIMEOUT 30
#define IDLE_TIMEOUT 60
#define READ_BYTES 2048

class Response;

struct t_client {
    int fd;

    std::string ip;

    time_t last_activity;
    time_t connection_time;

    ssize_t writeOffset;
    std::string outBuffer;

    ServerConfig *config;
    std::vector<ServerConfig *> server;

    HttpParser::httpResult req_res;
    HttpParser::httpResult resp_res;

    HttpParser *request;
    Response *response;
    void printInfo() const
    {
        std::cout << "t_client fd: " << this->fd << std::endl;
        std::cout << "t_client config address : " << this->config << std::endl;
        std::cout << "t_client request address : " << this->request << std::endl;
        std::cout << "t_client response address : " << this->response << std::endl;
    }
};

/*
 * server class depends on 'config' class with the
 * following getters: getListeningPairs() : get interface:port pairs on which the server will listen
 * to.
 */
class Server {
  private:
    Config conf;  // Config info
    int epoll_fd; // fd referring to the new epoll instance

    std::map<int, std::vector<ServerConfig *> > vServers;

    // std::vector<int> clientFds;
    // std::map<int, HttpParser *> clientFds;
    std::list<t_client> clients;

  public:
    Server(std::string &file);
    int startServer();
    void setupServerFds();
    int addToEpoll(int fd, uint32_t flag);
    int switchEpollMode(int clientSocket, uint32_t flag);
    int checkServerFd(epoll_event *events, int i);
    int checkClientFd(epoll_event *events, int i);
    void resetSocket(t_client *client);
    void closeSocket(t_client *client);

    int checkClientTimeout(t_client *);

    // int setup_epoll(config &conf);
    void printInfo() const
    {
        std::cout << "========== Server Info ==========" << std::endl;
        std::cout << "epoll_fd: " << this->epoll_fd << std::endl;
        std::cout << "clients: " << std::endl;
        for (std::list<t_client>::const_iterator it = this->clients.begin();
             it != this->clients.end(); it++) {
            std::cout << "t_client fd: " << it->fd << std::endl;
            std::cout << "t_client config address : " << it->config << std::endl;
            std::cout << "t_client request address : " << it->request << std::endl;
            std::cout << "t_client request response : " << it->response << std::endl;
        }
        // NOTE: ignore vServers for now
    }
};
#endif
