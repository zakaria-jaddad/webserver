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
	char pass[1024] = {0};
	const char* win = "Congrats!\n";
	int ret;
	int len;
	int b;
	std::string msg;
	std::ostringstream s;

	clientSocket = accept(serverSocket, NULL, NULL);

	send(clientSocket, "Welcome to flag guesser!\na very guessy chall where u learn nothing at all. Enjoy!\n", 83, 0);
	while (1) {
		send(clientSocket, "Guess the flag : ", 17, 0);
		b = recv(clientSocket, pass, sizeof(pass), 0);
		pass[b] = 0;
		std::cout << "Recvd: --" << pass << "--" << std::endl;
		ret = strcmp(pass, "CP_Z3R0{eezzzz_fl4G_F0r_u}\n");
		len = strlen(pass);
		if (ret == 0) {
			send(clientSocket, win, strlen(win), 0);
			break;
		}
		else {
			send(clientSocket, "haha gl next time\n", 18, 0);
			std::cout << "pass len = " << strlen(pass) << std::endl;
			std::cout << pass << std::endl;
			if (len < 27)
				send(clientSocket, "flag too short\n", 15, 0);
			else if (len > 27)
				send(clientSocket, "flag too big\n", 13, 0);
			else {
				msg = "strcmp(yourInput, flag) returned : "; 
				s << ret;
				msg += s.str();
				s.str("");
				msg += "\n";
				send(clientSocket, msg.c_str(), msg.length(), 0);
			}
		}
	}
}

int main(void)
{
	startSocket();
	return 0;
}
