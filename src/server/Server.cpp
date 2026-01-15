#include "./Server.hpp"
#include <sstream>

Server::Server(std::string &path) {
	this->conf.tokenize(path);
	this->conf.loadServers();
}
void Server::setupServerFds() {
	//TODO:iterate over ip:port pairs in conf and do the following to each pair:
	//for now i'll use the following two
	std::map<const char *, int> listeningPairs;
	listeningPairs["10.12.6.4"] = 8069;
	listeningPairs["0.0.0.0"] = 8080;

	int serverSocket;
	int opt = 1;

	for (std::map<const char *, int>::iterator it = listeningPairs.begin(); it != listeningPairs.end(); it++) {
		serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in serverAddr;

		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(it->second);
		if (inet_pton(AF_INET, it->first, &serverAddr.sin_addr) != 1)
			std::cerr << "error inet_pton " << it->first << std::endl;
		if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
			std::cerr << "error setsockopt " << it->first << std::endl;
		if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
			std::cerr << "error binding " << it->first << std::endl;
		if (listen(serverSocket, 10) == -1) //TODO: update later to get it from conf
			std::cerr << "on listen " << it->first << std::endl;
		//std::cout << "added socker fd : " << serverSocket << std::endl;
		serverFds.push_back(serverSocket);
	}
}

int Server::addToEpoll(int fd, uint32_t flag) {
	//This method is to add fd to epoll list of interest
	// with flag meaning that we are interested in read operation or write ...
	epoll_event_t event;

	event.events = flag;
	event.data.fd = fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
		std::cerr << "epoll_ctl Failed!" << std::endl;
		return (1);
	}
	return (0);
}

int Server::switchEpollMode(int clientSocket, uint32_t flag) {
	//This method is to switch epoll mode (read or write)
	epoll_event_t event;

	event.events = flag;
	event.data.fd = clientSocket;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, clientSocket, &event) == -1) {
		std::cerr << "epoll_ctl Failed!" << std::endl;
		return (1);
	}
	return (0);
}

int Server::checkForServerFd(epoll_event *events, int i) {
	//This method is just to look for fd in serverFds
	// in case it is a serverFd it accepts and gets clientSocket and adds it to epoll & clientFds for next connection
	int clientSocket = -1;
	HttpParser *obj;

	for (std::vector<int>::iterator it = serverFds.begin(); it != serverFds.end(); it++) {
		if (events[i].data.fd == *it) {
			//std::cout << "the fd is : " << *it << std::endl;
			clientSocket = accept(*it, NULL, NULL);
			if (clientSocket == -1)
				std::cerr << "failed to accept" << std::endl;
			if (addToEpoll(clientSocket, EPOLLIN))
				std::cerr << "failed to add to epoll" << std::endl;
			obj = new HttpParser();
			clientFds.insert(std::make_pair(clientSocket, obj));
			//std::cout << "got the clientSocket = " << clientSocket << std::endl;
			break;
		}
	}
	return (clientSocket);
}

int Server::checkForClientFd(epoll_event *events, int i) {
	//This method is to look for the event fd in clientFds
	//It then checks if the operation that is is ready for is 'read' or 'write'
	// then reads or writes accordingly

	const char* resp = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 7\r\n\r\nBruh hh";
	int clientSocket = -1;
	char req[20] = {0};
	int b;

	for (std::map<int, HttpParser *>::iterator it = clientFds.begin(); it != clientFds.end(); it++) {
		if (events[i].data.fd == it->first) {
			clientSocket = it->first;
			//std::cout << "already in map clientSocket = " << clientSocket << std::endl;
			if (events[i].events == EPOLLIN) { // we can read from socket
				b = recv(clientSocket, req, sizeof(req) - 1, 0);
				if (b == -1) {
					std::cerr << "failed to read" << std::endl;
					break;
				}
				req[b] = 0;
				std::cout << "----------------------start of request--------------------" << std::endl;
				std::cout << req;
				std::cout << "----------------------end of request--------------------" << std::endl;
				it->second->parseHttpRequest(req);
				//TODO: check if the req ended
				if (it->second->isComplete())
					switchEpollMode(clientSocket, EPOLLOUT);
			}
			else if (events[i].events == EPOLLOUT) { // we can write to socket
				if (send(clientSocket, resp, strlen(resp), 0) == -1) {
					std::cerr << "failed to send" << std::endl;
					break;
					//maybe close connection on error
				}
				//TODO: check if the response has ended
				//assuming again that the resp has ended
				//check if connection close or keep alive to know whether to close or in case of keep alive clear data buff falgs etc..
				switchEpollMode(clientSocket, EPOLLIN);
			}
			break;
		}
	}
	return (clientSocket);
}

int Server::startServer() {
	//(void *)conf;

	setupServerFds();
	int clientSocket;
	int event_count;

	epoll_fd = epoll_create1(0);
	epoll_event_t events[MAX_EVENTS];

	for (std::vector<int>::iterator it = serverFds.begin(); it != serverFds.end(); it++) {
		if (addToEpoll(*it, EPOLLIN))
			return (1);
	}

	while (1) {
		std::cout << "Listening..." << std::endl;
		event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000);
		if (event_count == -1) {
			std::cerr << "epoll_wait Failed!" << std::endl;
			return (1);
		}
		//std::cout << "event_count = " << event_count << std::endl;
		for (int i = 0; i < event_count; i++) {
			clientSocket = checkForServerFd(events, i);
			if (clientSocket != -1) //found fd in serverFds
				continue;
			clientSocket = checkForClientFd(events, i);
			if (clientSocket == -1)
				std::cerr << "error clienSocket Not Found!" << std::endl;
		}
	}
	return (0);
}
