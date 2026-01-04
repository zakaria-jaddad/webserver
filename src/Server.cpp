#include "../includes/Server.hpp"
#include <sstream>


std::vector<int> Server::setupServerFds() {
	//TODO:iterate over ip:port pairs in conf and do the following to each pair:
	//for now i'll use the following two
	std::map<const char *, int> listeningPairs;
	listeningPairs["10.12.0.0"] = 8067;
	listeningPairs["0.0.0.0"] = 8069;
	//listeningPairs[inet_pton(AF_INET, "0.0.0.0", &serverAddr.sin_addr)] = 8080;
	//listeningPairs[inet_pton(AF_INET, "0.0.0.0", &serverAddr.sin_addr)] = 8069;

	int serverSocket;
	std::vector<int> serverFds;

	for (std::map<const char *, int>::iterator it = listeningPairs.begin(); it != listeningPairs.end(); it++) {

		serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in serverAddr;

		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(it->second);
		//serverAddr.sin_addr.s_addr = it->first;
		inet_pton(AF_INET, it->first, &serverAddr.sin_addr);
		bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
		listen(serverSocket, 10); //TODO: update later to get it from conf
		std::cout << "added socker fd : " << serverSocket << std::endl;
		serverFds.push_back(serverSocket);
	}
	return serverFds;
}

int Server::addToEpoll(int fd, uint32_t flag) {
	//TODO: check for errors
	epoll_event_t event;

	event.events = flag;
	event.data.fd = fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
	std::cout << "added to epol_ctl socket fd : " << fd << std::endl;
	return (0);
}

int Server::switchEpollMode(int clientSocket, uint32_t flag) {
	epoll_event_t event;

	event.events = flag;
	event.data.fd = clientSocket;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, clientSocket, &event);
	return (0);
}

void Server::startServer() {
	//(void *)conf;


	std::vector<int> serverFds = setupServerFds();
	std::vector<int> clientFds;
	int clientSocket;
	char req[1024] = {0};
	int b;
	const char* resp = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 7\r\n\r\nBruh hh";

	epoll_fd = epoll_create1(0);
	epoll_event_t events[MAX_EVENTS];//where the fuck do i get the max possible value? idk

	for (std::vector<int>::iterator it = serverFds.begin(); it != serverFds.end(); it++) {
		addToEpoll(*it, EPOLLIN);
	}

	int event_count;

	while (1) {
		event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000);
		//find the right socket fd here. when i implement multiple serverSockets
		std::cout << "event_count = " << event_count << std::endl;
		for (int i = 0; i < event_count; i++) {
			clientSocket = -1;
			for (std::vector<int>::iterator it = serverFds.begin(); it != serverFds.end(); it++) {
				if (events[i].data.fd == *it) {
					std::cout << "the fd is : " << *it << std::endl;
					clientSocket = accept(*it, NULL, NULL);//just in the case of new connection
					addToEpoll(clientSocket, EPOLLIN);
					clientFds.push_back(clientSocket);
					std::cout << "got the clientSocket = " << clientSocket << std::endl;
					break;
				}
			}
			if (clientSocket == -1) {
				for (std::vector<int>::iterator it = clientFds.begin(); it != clientFds.end(); it++) {
					if (events[i].data.fd == *it) {
						clientSocket = *it;
						std::cout << "already in map clientSocket = " << clientSocket << std::endl;
						if (events[i].events == EPOLLIN) {
							b = recv(clientSocket, req, sizeof(req), 0);
							req[b] = 0;
							std::cout << req;
							//TODO: check if the req ended
							//i'll assume the requat has ended for now
							switchEpollMode(clientSocket, EPOLLOUT);
						}
						else if (events[i].events == EPOLLOUT) {
							send(clientSocket, resp, strlen(resp), 0);
							//TODO: check if the response has ended
							//assuming again it the resp has ended
							switchEpollMode(clientSocket, EPOLLIN);
						}
						break;
					}
				}
			}
		}
	}
}
