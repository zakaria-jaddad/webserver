#ifndef FT_WEBSERVER_HPP
#define FT_WEBSERVER_HPP

#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <vector>

class Server {
	/* server class depends on 'config' class with the following getters:
		- getListeningPairs() //get interface:port pairs on which the server will listen to.
			(type of return : map of 'int key' (interface) and 'int value' (port) )
	*/
	private:
		//config *conf;
	public:
		void startServer(); // TODO: change it to take config *conf instead
		//vector<int> & getServerFds(config &c);
};
#endif
