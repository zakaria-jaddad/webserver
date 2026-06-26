#include "../../includes/utils/utils.hpp"

bool isCgi(const std::string &uri, std::pair<std::string, std::string> cgiInfo)
{
    bool cgi_configured = cgiInfo.first.empty() == true || cgiInfo.second.empty() == true;

    if (!cgi_configured)
        return false;

    bool is_php_request = false;

    std::stringstream s(uri);
    std::string elem;
    std::vector<std::string> elements;

    while (getline(s, elem, '/')) {
        if (elem.empty())
            continue;
        elements.push_back(elem);
    }

    std::vector<std::string>::iterator it = elements.begin();

    for (; it != elements.end(); it++) {
        if ((*it).find(".php") != std::string::npos) {
            is_php_request = true;
            break;
        }
    }

    return is_php_request && cgi_configured;
}
