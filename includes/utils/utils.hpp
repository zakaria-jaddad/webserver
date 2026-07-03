#ifndef UTILS_HPP
#define UTILS_HPP

#include <map>
#include <sstream>
#include <string>
#include <vector>

bool isCgi(const std::string &uri, std::map<std::string, std::string> cgiInfo);
std::string getExtension(std::string &path);
bool isMethodAllowed(std::vector<std::string> acceptedMethods, int reqType);

#endif
