#include "../includes/ft_webserver.hpp"
#include <sstream>

void startSocket() {
	//(void *)conf;

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	listen(serverSocket, 5);

	int clientSocket;
	char req[1024] = {0};
	int b;
	const char* resp = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 7\r\n\r\nBruh hh";

	clientSocket = accept(serverSocket, NULL, NULL);

	while (1) {
		b = recv(clientSocket, req, sizeof(req), 0);
		req[b] = 0;
		std::cout << req;
		send(clientSocket, resp, strlen(resp), 0);
	}
}

int main(void)
{
	startSocket();
	return 0;
}
