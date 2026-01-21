#ifndef ENVUTILS_HPP
# define ENVUTILS_HPP

#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>
#include <map>

std::string								methodToString( HttpParser::requestType method );
std::vector<std::string>	deriveScriptName( std::string& path );
std::string								squashMapIntoString( std::map<std::string, std::string>& map);
std::string								joinHeaderValues(std::vector<std::string>& field_value);
std::string 							normalizeHeaderName(const std::string& name);
void 											headerEnvBuilder( std::map<std::string, std::string>& env, std::map<std::string, std::vector<std::string>>& header );

#endif