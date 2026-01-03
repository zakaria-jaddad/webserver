#include "server.hpp"

std::string helloRoute =
    "HTTP/1.1 200 OK\r\nContent-Type: "
    "text/html\r\nContent-Length: 21\r\n\r\n<h1>Hello, World</h1>";

void Server::startServer() {
  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in serverAddr;
  bool enabled = true;

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(9999);
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  int rc =
      bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
  if (rc < 0) {
    std::cout << "bind() error still don't know why?" << std::endl;
    close(serverSocket);
    return;
  }
  listen(serverSocket, 5);

  int clientSocket;
  char req[1024] = {0};
  int b;

  (void)setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enabled,
                   sizeof(enabled));

  clientSocket = accept(serverSocket, NULL, NULL);

  for (;;) {
    b = recv(clientSocket, req, sizeof(req), 0);
    if (b > 0) {
      req[b] = 0;

      /* std::cout << req; */
      HttpParser parser;
      http_result res = parser.parseHttpRequest(req);
      if (res != HTTP_RES_OK) {
        /* TODO: Add correct return */
        /* something happened while parsing ignored for now */
      } else {
        std::cout << parser.getRequestType() << std::endl;
        std::cout << parser.getRequestURI() << std::endl;
        std::cout << parser.getRequestVersion() << std::endl;
        std::map<std::string, std::string> headers = parser.getHeaders();
        std::map<std::string, std::string>::iterator it;

        for (it = headers.begin(); it != headers.end(); it++) {
          std::cout << "{ " << it->first << " " << it->second << " }"
                    << std::endl;
        }
        send(clientSocket, helloRoute.c_str(), helloRoute.size(), 0);
      }
    }
  }
}
