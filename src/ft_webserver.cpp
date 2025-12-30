#include "../includes/ft_webserver.hpp"

void startSocket() {
	//(void *)conf;

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	listen(serverSocket, 5);

	int clientSocket = accept(serverSocket, NULL, NULL);

	char buffer[1024] = {0};
	recv(clientSocket, buffer, sizeof(buffer), 0);
	std::cout << "Recvd: " << buffer << std::endl;
	const char* txt = "CP_Z3R0{eezzzz_fl4G_F0r_u}";
	send(clientSocket, txt, 26, 0);
	close(serverSocket);
	close(clientSocket);
}

int main(void)
{
	startSocket();
	return 0;
}
