#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include "../request/request.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

typedef struct epoll_event epoll_event_t;
#define MAX_EVENTS 100

class Server {
    /* server class depends on 'config' class with the following getters:
            - getListeningPairs() //get interface:port pairs on which the server
       will listen to. (type of return : map of 'int key' (interface) and 'int
       value' (port) )
    */
  private:
    // config &conf = configObject;
  public:
    void startServer(); // TODO: change it to take config *conf instead
};
#endif
