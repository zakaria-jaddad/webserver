#include "../../includes/utils/utils.hpp"

bool isCgi(const std::string& uri, std::map<std::string, std::string> cgiInfo)
{
	bool cgi_configured = !(cgiInfo.empty());
												
	if (!cgi_configured)
		return false;
	
	bool is_cgi_request = false;

	std::stringstream s(uri);
	std::string elem;
	std::vector<std::string> elements;

	while (getline(s, elem, '/')) {
		if (elem.empty())
			continue;
		elements.push_back(elem);
	}

	std::vector<std::string>::iterator it = elements.begin();

	for (; it != elements.end(); it++)
	{
		if (((*it).find(".php") != std::string::npos) || ((*it).find(".py") != std::string::npos))
		{
			is_cgi_request = true;
			break;
		}
	}

	return is_cgi_request && cgi_configured;
}
