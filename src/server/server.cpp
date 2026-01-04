#include "server.hpp"

std::string helloRoute = "HTTP/1.1 200 OK\r\nContent-Type: "
                         "text/html\r\nContent-Length: 21\r\n\r\n<h1>Hello, World</h1>";

std::string foo = "HTTP/1.1 200 OK\r\nContent-Type: "
                  "text/html\r\nContent-Length: 21\r\n\r\n<h1>............</h1>";

void Server::startServer()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    bool enabled = true;

    (void)setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54321);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    int rc = bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (rc < 0) {
        std::cout << "bind() error still don't know why?" << std::endl;
        close(serverSocket);
        return;
    }
    listen(serverSocket, 5);

    int clientSocket;
    char req[1024] = {0};
    int b;

    clientSocket = accept(serverSocket, NULL, NULL);

    for (;;) {
        b = recv(clientSocket, req, sizeof(req), 0);
        if (b > 0) {
            req[b] = 0;

            std::cout << req;
            HttpParser parser;
            httpResult res = parser.parseHttpRequest(req);
            if (res != HTTP_RES_OK) {
                /* TODO: Add correct return */
                /* something happened while parsing ignored for now */

                send(clientSocket, foo.c_str(), foo.size(), 0);
            }
            else {
                send(clientSocket, helloRoute.c_str(), helloRoute.size(), 0);
            }
        }
    }
}
