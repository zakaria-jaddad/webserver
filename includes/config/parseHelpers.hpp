#ifndef PARSEHELPERS_HPP
# define PARSEHELPERS_HPP

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

std::string readFile(const std::string& path);
bool isAbsolutePath(std::string path);
bool isNumber(const std::string& s);
bool isValidIPv4(const std::string& ip);

#endif
