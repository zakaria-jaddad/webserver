#include "../../includes/cgi/Cgi.hpp"

std::vector<std::string> deriveScriptName( std::string& path )
{
	std::stringstream s(path);
	std::string elem;
	std::vector<std::string> elements;
	std::string script_name;
	std::string path_info;

	while (getline(s, elem, '/')) {
		if (elem.empty())
			continue;
		elements.push_back(elem);
	}

	std::vector<std::string>::iterator it = elements.begin();

	for (; it != elements.end(); it++) 
	{
		script_name += "/";
		script_name += *it;
		if ((*it).find(".php") != std::string::npos)
		{
			it++;
			break;
		}
	}

	for (; it != elements.end(); it++)
	{
		path_info += "/";
		path_info += *it;
	}

	elements.clear();
	
	elements[0] = script_name;
	elements[1] = path_info;

	return elements;
}

std::string methodToString( HttpParser::requestType method )
{
	std::string res;

	if (method == HttpParser::GET)
		res = "GET";
	else if (method == HttpParser::DELETE)
		res = "DELETE";
	else if (method == HttpParser::POST)
		res = "POST";

	return res;
}

std::string squashMapIntoString( std::map<std::string, std::string>& map)//tmp
{
	std::string	env_val = "";
	
	for (std::map<std::string, std::string>::iterator it; it != map.end(); it++)
	{
		env_val += it->first + "=" + it->second + ", ";
	}

	return env_val;
}

std::string	joinHeaderValues(std::vector<std::string>& field_value)
{
	std::string	env_val;
	
	for (size_t i = 0; i < field_value.size(); i++)
	{
		env_val += field_value[i] + ", ";
	}

	return env_val;
}

std::string normalizeHeaderName(const std::string& name)
{
	std::string header = "HTTP_" + name;

	for (size_t i = 0; i < header.length(); i++)
	{
		char c = header[i];

		if (c == '-')
			header[i] = '_';
		if (std::isalpha(c))
			header[i] = std::toupper(c);
	}

	return header;
}

void headerEnvBuilder( std::map<std::string, std::string>& env, std::map<std::string, std::vector<std::string> >& header )
{
	std::map<std::string, std::vector<std::string> >::iterator it = header.begin();

	for (; it != header.end(); it++)
	{
		std::string	field_value = joinHeaderValues(it->second);
		std::string	field_name = normalizeHeaderName(it->first);
		env[field_name] = field_value;
	}
}