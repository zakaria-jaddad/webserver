#ifndef SERVER_HPP
#define SERVER_HPP

#include "../../includes/config/Config.hpp"
#include "../../includes/request/Request.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

typedef struct epoll_event epoll_event_t;
#define MAX_EVENTS 100

typedef struct {
    int fd;
    ServerConfig *config;
    HttpParser *request;
} t_client;

class Server {
    /* server class depends on 'config' class with the following getters:
            - getListeningPairs() //get interface:port pairs on which the server will listen to.
                    (type of return : map of 'int key' (interface) and 'int value' (port) )
    */
  private:
    Config conf;
    int epoll_fd;
    // std::vector<int> serverFds;
    std::map<int, ServerConfig *> vServers;
    // std::vector<int> clientFds;
    // std::map<int, HttpParser *> clientFds;
    std::vector<t_client> Clients;

  public:
    Server(std::string &file);
    int startServer();
    void setupServerFds();
    int addToEpoll(int fd, uint32_t flag);
    int switchEpollMode(int clientSocket, uint32_t flag);
    int checkForServerFd(epoll_event *events, int i);
    int checkForClientFd(epoll_event *events, int i);
    // int setup_epoll(config &conf);
};
#endif
