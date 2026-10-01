#include "../../includes/utils/utils.hpp"

ServerConfig *resolveServer(const std::vector<ServerConfig *> &serverConfigs,
                            const std::string &hostHeader)
{
    if (serverConfigs.empty())
        return NULL;

    if (hostHeader.empty())
        return serverConfigs[0]; // or NULL / bad request, depending on your policy

    std::string host = hostHeader;

    // strip port if present: example.com:8080 -> example.com
    size_t colon = host.find(':');
    if (colon != std::string::npos)
        host = host.substr(0, colon);

    // normalize to lowercase
    for (size_t i = 0; i < host.size(); ++i)
        host[i] = std::tolower(static_cast<unsigned char>(host[i]));

    // exact match
    for (std::vector<ServerConfig *>::const_iterator srv = serverConfigs.begin();
         srv != serverConfigs.end(); ++srv) {
        std::vector<std::string> names = (*srv)->getServerName();
        for (std::vector<std::string>::const_iterator name = names.begin(); name != names.end();
             ++name) {
            if (*name == host)
                return *srv;
        }
    }

    // fallback to first config
    return serverConfigs[0];
}
