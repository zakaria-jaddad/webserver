#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <cstddef>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#define CRLF "\r\n"
#define DOUBLEC_RLF "\r\n\r\n"

typedef enum {
  HTTP_RES_OK = 200,
  HTTP_RES_INTERNAL_SERVER_ERROR = 500,
  HTTP_RES_BAD_REQUEST = 400,
} http_result;

class HttpParser {

private:
  std::string _requestType;
  std::string _requestURI;
  std::string _requestVersion;
  std::map<std::string, std::string> _headers;

  std::vector<std::string> getTokens(std::string line, std::string delimiter) {
    size_t pos = 0;
    std::vector<std::string> tokens;
    std::string token;
    while ((pos = line.find(delimiter)) != std::string::npos) {
      token = line.substr(0, pos);
      tokens.push_back(token);
      line.erase(0, pos + delimiter.length());
    }
    tokens.push_back(line);
    return tokens;
  }

  // Request-Line = Method SP Request-URI SP HTTP-Version CRLF
  http_result parseRequestLine(std::string requestLine) {

    std::vector<std::string> tokens = getTokens(requestLine, " ");
    if (tokens.size() != 3) {
      return HTTP_RES_BAD_REQUEST;
    }
    this->_requestType = tokens[0];
    this->_requestURI = tokens[1];
    this->_requestVersion = tokens[2];
    return HTTP_RES_OK;
  }

  http_result parseRequestHeaders(std::string headerLines) {

    std::cout << headerLines << std::endl;
    std::vector<std::string> tokens = getTokens(headerLines, " ");
    size_t size = tokens.size();

    // i + 2 since headers are a key value paire
    // [key, value, key, value, ...]
    for (int i = 0; i < int(size); i = i + 2) {
      this->_headers.insert(std::make_pair(tokens[i], tokens[i + 1]));
    }
    return HTTP_RES_OK;
  }

public:
  HttpParser() {
    this->_requestType = "";
    this->_requestURI = "";
    this->_requestVersion = "";
  }

  ~HttpParser() {}

  http_result parseHttpRequest(std::string rawData) {

    std::string requestLine = rawData.substr(0, rawData.find(CRLF));
    rawData.erase(0, requestLine.size() + strlen(CRLF));

    if (parseRequestLine(requestLine) != HTTP_RES_OK)
      return (HTTP_RES_INTERNAL_SERVER_ERROR);

    std::string headersLines = rawData.substr(0, rawData.find(DOUBLEC_RLF));
    rawData.erase(0, requestLine.size() + strlen(DOUBLEC_RLF));

    if (parseRequestHeaders(headersLines) != HTTP_RES_OK)
      return (HTTP_RES_INTERNAL_SERVER_ERROR);

    return HTTP_RES_OK;
  };

  /* TODO: getters should return a reference */
  std::string getRequestType() const { return this->_requestType; }
  std::string getRequestURI() const { return this->_requestURI; }
  std::string getRequestVersion() const { return this->_requestVersion; }
  std::map<std::string, std::string> getHeaders() const {
    return this->_headers;
  }
};
#endif // !__REQUEST_HPP__
