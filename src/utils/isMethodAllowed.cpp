#include "../../includes/utils/utils.hpp"
#include "../../includes/request/Request.hpp"

bool isMethodAllowed(std::vector<std::string> acceptedMethods, int reqType)
{
    HttpParser::requestType type = static_cast<HttpParser::requestType>(reqType);

    for (size_t i = 0; i < acceptedMethods.size(); i++) {
        if (HttpParser::getRequestTypeFromString(acceptedMethods[i]) == type) {
            return true;
        }
    }
    return false;
}
