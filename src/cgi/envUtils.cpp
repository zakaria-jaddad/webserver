#include "../../includes/cgi/Cgi.hpp"

std::string	joinHeaderValues(std::vector<std::string>& field_value)
{
	std::string	env_val;
	
	for (int i = 0; i < field_value.size(); i++)
	{
		env_val += field_value[i] + ", ";
	}

	return env_val;
}

std::string normalizeHeaderName(const std::string& name)
{
	std::string header = "HTTP_" + name;

	for (int i = 0; i < header.length(); i++)
	{
		char c = header[i];

		if (c == '-')
			header[i] = '_';
		if (std::isalpha(c))
			header[i] = std::toupper(c);
	}

	return header;
}

void headerEnvBuilder( std::map<std::string, std::string>& env, std::map<std::string, std::vector<std::string>>& header )
{
	std::map<std::string, std::vector<std::string>>::iterator it = header.begin();

	for (; it != header.end(); it++)
	{
		std::string	field_value = joinHeaderValues(it->second);
		std::string	field_name = normalizeHeaderName(it->first);
		env[field_name] = field_value;
	}
}