#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <vector>
#include <map>
#include <arpa/inet.h>
#include <fcntl.h>

typedef struct epoll_event epoll_event_t;
#define MAX_EVENTS 100

class Server {
	/* server class depends on 'config' class with the following getters:
		- getListeningPairs() //get interface:port pairs on which the server will listen to.
			(type of return : map of 'int key' (interface) and 'int value' (port) )
	*/
	private:
		//config &conf = configObject;
		int epoll_fd;
		std::vector<int> serverFds;
		std::vector<int> clientFds;
	public:
		int startServer(); // TODO: change it to take config *conf instead
		void setupServerFds();// TODO: change it to take config *conf instead
		int addToEpoll(int fd, uint32_t flag);
		int switchEpollMode(int clientSocket, uint32_t flag);
		int checkForServerFd(epoll_event *events, int i);
		int checkForClientFd(epoll_event *events, int i);
		//int setup_epoll(config &conf);
};
#endif
