#ifndef SERVER_HPP
#define SERVER_HPP

#include "../config/Config.hpp"
#include "../request/Request.hpp"
#include "../response/Response.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <deque>
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
#define REQUEST_TIMEOUT 30
#define IDLE_TIMEOUT 60
#define READ_BYTES 2048

class Response;

struct t_client {
    int fd;

    time_t	last_activity;
	time_t	connection_time;

    ServerConfig *config;
    HttpParser::httpResult req_res;// check later should be changed and included in the request & resp objects themselves
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
    std::deque<t_client> clients;

  public:
    Server(std::string &file);
    int startServer();
    void setupServerFds();
    int addToEpoll(int fd, uint32_t flag);
    int switchEpollMode(int clientSocket, uint32_t flag);
    int checkForServerFd(epoll_event *events, int i);
    int checkForClientFd(epoll_event *events, int i);
    void resetSocket(t_client *client);
    void closeSocket(t_client *client);

    int checkClientTimeout( t_client* );

    // int setup_epoll(config &conf);
    void printInfo() const
    {
        std::cout << "========== Server Info ==========" << std::endl;
        std::cout << "epoll_fd: " << this->epoll_fd << endl;
        std::cout << "clients: " << std::endl;
        for (std::deque<t_client>::const_iterator it = this->clients.begin();
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
