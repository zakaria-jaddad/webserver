#include "server.hpp"

void Server::startServer() {
  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in serverAddr;

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(2010);
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  int rc =
      bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
  if (rc < 0) {
    close(serverSocket);
    return;
  }
  listen(serverSocket, 5);

  int clientSocket;
  char req[1024] = {0};
  int b;
  std::string resp =
      "HTTP/1.1 200 OK\r\nContent-Type: "
      "text/html\r\nContent-Length: 21\r\n\r\n<h1>Hello, World</h1>";

  clientSocket = accept(serverSocket, NULL, NULL);

  for (;;) {
    b = recv(clientSocket, req, sizeof(req), 0);
    req[b] = 0;
    std::cout << req;
    send(clientSocket, resp.c_str(), resp.size(), 0);
  }
}
