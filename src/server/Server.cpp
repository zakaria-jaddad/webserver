#include "../../includes/server/Server.hpp"
#include <sstream>

Server::Server(std::string &path) {
	this->conf.tokenize(path);
	this->conf.loadServers();
}

void Server::setupServerFds() {
	int serverSocket;
	int opt = 1;

	for (std::vector<ServerConfig>::iterator it = this->conf.servers.begin(); it != this->conf.servers.end(); it++) {
		serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in serverAddr;

		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(it->getPort());
		if (inet_pton(AF_INET, it->getIP().c_str(), &serverAddr.sin_addr) != 1)
			std::cerr << "error inet_pton " << it->getIP() << std::endl;
		if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
			std::cerr << "error setsockopt " << it->getIP() << std::endl;
		if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
			std::cerr << "error binding " << it->getIP() << std::endl;
		if (listen(serverSocket, 10) == -1) //TODO: update later to get it from conf
			std::cerr << "on listen " << it->getIP() << std::endl;
		//std::cout << "added socker fd : " << serverSocket << std::endl;
		this->vServers[serverSocket] = &(*it);
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
	t_client client;
	time_t now = time(NULL);

	for (std::map<int, ServerConfig *>::iterator it = this->vServers.begin(); it != this->vServers.end(); it++) {
		if (events[i].data.fd == it->first) {
			clientSocket = accept(it->first, NULL, NULL);
			if (clientSocket == -1)
				std::cerr << "failed to accept" << std::endl;
			if (addToEpoll(clientSocket, EPOLLIN))
				std::cerr << "failed to add to epoll" << std::endl;
			obj = new HttpParser();
			client.fd = clientSocket;
			client.config = it->second;
			client.request = obj;
			client.response = NULL;
			client.connection_time = now;
			client.last_activity = now;
			Clients.push_back(client);
			break;
		}
	}
	return (clientSocket);
}

void	Server::resetSocket(t_client *client) {
	delete client->request;
	delete client->response;
	client->request = new HttpParser();
	client->response = NULL;
}

void	Server::closeSocket(t_client *client) {
	delete client->request;
	delete client->response;
	//delete client->config;
	close(client->fd);
	client->request = NULL;
	client->response = NULL;
	client->config = NULL;
}

int Server::checkClientTimeout(t_client *client)
{
	time_t now = time(NULL);
	time_t idle_time = now - client->last_activity;
	
	std::cout << "[" << idle_time << "]" << "idle value for fd=" << client->fd << std::endl;
	std::cout << "[" << now << "]" << "checking timeout..." << std::endl;

	if (!client->request->isComplete() && idle_time > REQUEST_TIMEOUT)//placeholder request timeout
		return 1;
	
	if (client->request->isComplete() && idle_time > IDLE_TIMEOUT)//placeholder idle timeout
		return 1;
	
	return 0;
}

int Server::checkForClientFd(epoll_event *events, int i) {
	//This method is to look for the event fd in clientFds
	//It then checks if the operation that is is ready for is 'read' or 'write'
	// then reads or writes accordingly

	int clientSocket = -1;
	char req[1024] = {0}; 
	int b;
	int c = 0;

	for (std::vector<t_client>::iterator it = this->Clients.begin(); it != this->Clients.end(); it++, c++) 
	{
		std::cout << "looping thru clients" << std::endl;

		int isTimeout = checkClientTimeout(&(*it));
		if (isTimeout)
		{
			std::cout << "client fd=" << (*it).fd << " timed out " << std::endl;

			if (it->response == NULL)
				it->response = new Response(&(*it));

			(*it).response->servePages(408);

			closeSocket(&(*it));
			switchEpollMode((*it).fd, EPOLL_CTL_DEL);
			Clients.erase(Clients.begin() + c);
			return -2;
		}
		else
			clientSocket = 0;

		if (events[i].data.fd == it->fd) 
		{
			clientSocket = it->fd;
			//std::cout << "already in map clientSocket = " << clientSocket << std::endl;
			if (events[i].events == EPOLLIN) { // we can read from socket
				b = recv(clientSocket, req, sizeof(req) - 1, 0);
				if (b == -1 || b == 0) {
					closeSocket(&(*it));
					Clients.erase(Clients.begin() + c);
					return 1;
					//TODO:check if i must remove from epoll list of interest
				}
				req[b] = 0;
				it->last_activity = time(NULL);
				std::cout << "--------------start of request----------------" << std::endl;
				std::cout << req;
				std::cout << "--------------end of request------------------" << std::endl;
				it->request->parseHttpRequest(req);
				
				//debuggin parsed request
				std::cout << "=======================" << std::endl;
				std::map<std::string, std::vector<std::string> > foo = it->request->getHeaders();

				for (std::map<std::string, std::vector<std::string> >::iterator itMap = foo.begin();
					itMap != foo.end();
					++itMap)
				{
					std::string key = itMap->first;
					std::vector<std::string>& values = itMap->second;

					std::cout << "Header: " << key << std::endl;

					for (std::vector<std::string>::iterator itVec = values.begin();
						itVec != values.end();
						++itVec)
					{
						std::cout << "  Value: " << *itVec << std::endl;
					}
				}
				std::cout << "=======================" << std::endl;
				
				if (it->request->isComplete())
					switchEpollMode(clientSocket, EPOLLOUT);
			}
			else if (events[i].events == EPOLLOUT) { // we can write to socket
				if (it->response == NULL)
					it->response = new Response(&(*it));
				it->response->checkMethod();
				if (it->response->isError()) {
					closeSocket(&(*it));
					Clients.erase(Clients.begin() + c);
					break;
					//hh
				}
				if (it->response->isComplete()) {
					resetSocket(&(*it));
					switchEpollMode(clientSocket, EPOLLIN);
					std::cout << "back to reading" << std::endl;
				}
				//check if connection close or keep alive to know
				//whether to close or in case of keep alive clear data buff falgs etc..
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

	for (std::map<int, ServerConfig *>::iterator it = this->vServers.begin(); it != this->vServers.end(); it++) {
		if (addToEpoll(it->first, EPOLLIN))
			return (1);
	}

	while (1) {
		event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);

		if (event_count == -1) {
			std::cerr << "epoll_wait Failed!" << std::endl;
			return (1);
		}
		// std::cout << "event_count = " << event_count << std::endl;
		if (event_count > 0)
			event_count -= 1;
		for (int i = 0; i <= event_count; i++) {
			std::cout << "i : " << i << std::endl;
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
