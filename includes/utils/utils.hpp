#ifndef UTILS_HPP
#define UTILS_HPP

#include "../config/ServerConfig.hpp"
#include <map>
#include <sstream>
#include <string>
#include <vector>

bool isCgi(const std::string &uri, std::map<std::string, std::string> cgiInfo);
std::string getExtension(std::string &path);
bool isMethodAllowed(std::vector<std::string> acceptedMethods, int reqType);
ServerConfig *resolveServer(const std::vector<ServerConfig *> &serverConfigs,
                            const std::string &hostHeader);

#endif
