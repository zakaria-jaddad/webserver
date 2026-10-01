#include "../../includes/config/LocationConfig.hpp"
#include "../../includes/config/parseHelpers.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

LocationConfig::LocationConfig()
    : cgi_info(), index(), methods_accepted(), redir(), root(""), upload_path(""),
      location_path(""), directory_listing(false), max_body(1024)
{
    is_set_cgi_info = false;
    is_set_index = false;
    is_set_methods_accepted = false;
    is_set_redir = false;
    is_set_root = false;
    is_set_upload_path = false;
    is_set_location_path = false;
    is_set_directory_listing = false;
    is_set_body_size = false;
    buildLocationParsers();
}

void LocationConfig::buildLocationParsers()
{
    locationParsers["root"] = &LocationConfig::parseRoot;
    locationParsers["index"] = &LocationConfig::parseIndex;
    locationParsers["upload_store"] = &LocationConfig::parsePath;
    locationParsers["autoindex"] = &LocationConfig::parseListing;
    locationParsers["return"] = &LocationConfig::parseRedir;
    locationParsers["client_max_body_size"] = &LocationConfig::parseSize;
    locationParsers["allow"] = &LocationConfig::parseMethods;
    locationParsers["cgi"] = &LocationConfig::parseCgi;
}

void LocationConfig::parseBlock(std::vector<std::string> &tokens, size_t &i)
{
    // skip location token
    i++;

    std::string loc_path = tokens[i];

    if (!isAbsolutePath(tokens[i++]) || tokens[i++] != "{")
        throw std::runtime_error("Incorrect location block");

    this->location_path = loc_path;

    while (tokens[i] != "}") {
        std::map<std::string, Parser>::iterator it = locationParsers.find(tokens[i]);

        if (it == locationParsers.end())
            throw std::runtime_error("Unknown directive: " + tokens[i]);

        Parser parser = it->second;
        (this->*parser)(tokens, i);
    }
    i++;
}

// parsing root directive
void LocationConfig::parseRoot(std::vector<std::string> &tokens, size_t &i)
{
    // skip the directive name token
    i++;

    if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
        throw std::runtime_error("Found an unexpected : " + tokens[i]);

    std::string value = tokens[i];

    // skip the value token
    i++;

    if (tokens[i] != ";")
        throw std::runtime_error("Invalid number of argumenst in \"root\" directive");

    // skip the closing ';'
    i++;

    for (size_t i = 0; i < value.size(); ++i) {
        if (!std::isalnum(value[i]) && value[i] != '-' && value[i] != '.' && value[i] != '/' &&
            value[i] != '_')
            throw std::runtime_error("Invalid root directory : " + value);
    }
    this->root = value;
    this->is_set_root = true;
}

// parsing index directive
void LocationConfig::parseIndex(std::vector<std::string> &tokens, size_t &i)
{
    // skip the directive name token
    i++;

    if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
        throw std::runtime_error("Found an unexpected : " + tokens[i]);

    while (tokens[i] != ";") {
        std::string value = tokens[i];
        for (size_t y = 0; y < value.size(); y++) {
            if (!std::isalnum(value[y]) && value[y] != '-' && value[y] != '.' && value[y] != '_')
                throw std::runtime_error("Invalid index  : " + value);
        }
        this->index.push_back(value);
        i++;
    }
    this->is_set_index = true;

    // skip closing ;
    i++;
}

// parsing autoindex directive
void LocationConfig::parseListing(std::vector<std::string> &tokens, size_t &i)
{
    // skip the directive name token
    i++;

    if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
        throw std::runtime_error("Found an unexpected : " + tokens[i]);

    std::string value = tokens[i];

    // skips value
    i++;

    if (tokens[i] != ";")
        throw std::runtime_error("Invalid number of argumenst in \"autoindex\" directive");

    // skips closing ;
    i++;

    if (value == "on")
        this->directory_listing = true;
    else if (value == "off")
        this->directory_listing = false;
    else
        throw std::runtime_error("Invalid value for : \"autoindex\" directive : " + value);

    this->is_set_directory_listing = true;
}

// parsing cgi directive
void LocationConfig::parseCgi(std::vector<std::string> &tokens, size_t &i)
{
    // skip the directive name token
    i++;
    // cgi .php /usr/bin/php-cgi
    if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
        throw std::runtime_error("Found an unexpected : " + tokens[i]);

    std::string extension = tokens[i];
    i++;

    if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
        throw std::runtime_error("Found an unexpected : " + tokens[i]);

    std::string cgi_path = tokens[i];

    // skipping path
    i++;

    if (tokens[i] != ";")
        throw std::runtime_error("Found an unexpected : '" + tokens[i] + "' When expecting ';'");

    i++;

    if (extension != ".php" && extension != ".py")
        throw std::runtime_error("Invalid extension : " + extension);

    if (!isAbsolutePath(cgi_path))
        throw std::runtime_error("Invalid path : " + cgi_path);

    this->cgi_info.insert(std::make_pair(extension, cgi_path));

    this->is_set_cgi_info = true;
}

// parsing allow directive
void LocationConfig::parseMethods(std::vector<std::string> &tokens, size_t &i)
{
    // skip the directive name token
    i++;

    if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
        throw std::runtime_error("Found an unexpected : " + tokens[i]);

    std::string methods[] = {"GET", "POST", "DELETE", "HEAD", "PUT", "OPTIONS", "TRACE", "CONNECT"};

    while (tokens[i] != ";") {
        std::string value = tokens[i];
        if (std::find(methods, methods + 8, value) == (methods + 8))
            throw std::runtime_error("Found an unexpected method : " + value);
        this->methods_accepted.push_back(value);
        i++;
    }

    this->is_set_methods_accepted = true;
    // skip closing ;
    i++;
}

// parsing index directive
void LocationConfig::parsePath(std::vector<std::string> &tokens, size_t &i)
{
    // skip the directive name token
    i++;

    if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
        throw std::runtime_error("Found an unexpected : " + tokens[i]);

    std::string value = tokens[i];

    // skips value
    i++;

    if (tokens[i] != ";")
        throw std::runtime_error("Invalid number of arguments in \"upload_store\" directive");

    // skips closing ;
    i++;

    if (!isAbsolutePath(value))
        throw std::runtime_error("Invalid upload path : " + value);

    this->upload_path = value;

    this->is_set_upload_path = true;
}

// parsing index directive
void LocationConfig::parseRedir(std::vector<std::string> &tokens, size_t &i)
{
    // skip the directive name token
    i++;

    std::string value = tokens[i];

    // skipping value
    i++;

    if (value == ";" || value == "{" || value == "}" || tokens[i] == ";" || tokens[i] == "{" ||
        tokens[i] == "}")
        throw std::runtime_error("Found an unexpected : " + value);

    std::string path = tokens[i];

    // skipping path
    i++;

    if (!isNumber(value) || value.size() != 3 || !isRedirectCode(std::atoi(value.c_str())))
        throw std::runtime_error("Invalid return code");

    if (!isAbsolutePath(path))
        throw std::runtime_error("Invalid redirection path");

    int number = std::atoi(value.c_str());

    this->redir = std::make_pair(number, path);
    this->is_set_redir = true;

    // skip closing ;
    i++;
}

// client_max_body_size directive parsing
std::size_t LocationConfig::checkSize(std::string &value)
{
    if (value.empty())
        throw std::runtime_error("client_max_body_size: empty value");

    char suffix = value[value.size() - 1];
    std::size_t multiplier = 1;
    std::string numberPart = value;

    if (!std::isdigit(static_cast<unsigned char>(suffix))) {
        if (suffix == 'k' || suffix == 'K')
            multiplier = 1024;
        else if (suffix == 'm' || suffix == 'M')
            multiplier = 1024 * 1024;
        else if (suffix == 'g' || suffix == 'G')
            multiplier = 1024 * 1024 * 1024;
        else
            throw std::runtime_error("client_max_body_size: invalid suffix");

        numberPart = value.substr(0, value.size() - 1);
    }

    if (numberPart.empty())
        throw std::runtime_error("client_max_body_size: invalid value");

    if (!isNumber(numberPart))
        throw std::runtime_error("client_max_body_size: invalid number");

    std::size_t size = static_cast<std::size_t>(std::atoi(numberPart.c_str()));

    if (size == 0)
        throw std::runtime_error("client_max_body_size: must be greater than zero");

    return size * multiplier;
}

void LocationConfig::parseSize(std::vector<std::string> &tokens, size_t &i)
{
    // skip the directive name token
    i++;

    if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
        throw std::runtime_error("Found an unexpected : " + tokens[i]);

    std::string value = tokens[i];
    // skip the value token
    i++;

    if (tokens[i] != ";")
        throw std::runtime_error(
            "Invalid number of arguments in \"client_max_body_size\" directive");

    // skip the closing ';'
    i++;

    this->max_body = checkSize(value);
    this->is_set_body_size = true;
}

// getters
std::vector<std::string> LocationConfig::getIndex(void) { return index; }

std::size_t LocationConfig::getSize(void) { return max_body; }

std::vector<std::string> LocationConfig::getAcceptedMethods(void) { return methods_accepted; }

std::pair<int, std::string> LocationConfig::getRedir(void) { return redir; }

std::string LocationConfig::getRoot(void) { return root; }

std::string LocationConfig::getUploadPath(void) { return upload_path; }

bool LocationConfig::getDirectoyListing(void) { return directory_listing; }

std::string LocationConfig::getLocationPath(void) { return location_path; }

std::map<std::string, std::string> LocationConfig::getCgi(void) { return cgi_info; }
