#ifndef PARSEHELPERS_HPP
# define PARSEHELPERS_HPP

#include <iostream>
#include <algorithm>

bool isAbsolutePath(std::string path);
bool isNumber(const std::string& s);
bool isValidIPv4(const std::string& ip);

#endif
