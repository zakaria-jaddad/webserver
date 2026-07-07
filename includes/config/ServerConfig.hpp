#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "LocationConfig.hpp"

class ServerConfig {

    std::map<int, std::string> error_pages;
    std::pair<int, std::string> redir;
    std::map<std::string, std::string> cgi_info;
    std::vector<std::string> index;
    std::vector<std::string> methods_accepted;
    std::vector<std::string> server_name;
    std::string ip;
    std::string root;
    std::size_t max_body;
    bool directory_listing; // only a fallback when index is not specified
    int port;

    typedef void (ServerConfig::*Parser)(std::vector<std::string> &, size_t &);
    std::map<std::string, Parser> serverParsers;

  public:
    std::vector<LocationConfig> location; // don't forget to make it private
    ServerConfig();

    std::size_t checkSize(std::string &);
    void buildServerParsers(void);
    void init_error_pages(void);

    void parseBlock(std::vector<std::string> &, size_t &);

    // directive parsing
    void parseName(std::vector<std::string> &, size_t &);
    void checkServerName(const std::string &);

    void parseListen(std::vector<std::string> &, size_t &);
    void parseIpOnly(const std::string &);
    void parseIpPort(const std::string &);
    void parsePortOnly(const std::string &);

    void parseIndex(std::vector<std::string> &, size_t &);

    void parseError(std::vector<std::string> &, size_t &);

    void parseSize(std::vector<std::string> &, size_t &);

    void parseRoot(std::vector<std::string> &, size_t &);

    void parseListing(std::vector<std::string> &, size_t &);
    void parseRedir(std::vector<std::string> &, size_t &);
    void parseMethods(std::vector<std::string> &, size_t &);
    void parseCgi(std::vector<std::string> &, size_t &);

    // utility methods

    LocationConfig *matchLocation(const std::string &);

    // getters
    std::map<std::string, int> getListeningPairs(void);
    std::map<int, std::string> getErrorPages(void);
    std::vector<std::string> getIndex(void);
    std::vector<std::string> getServerName(void);
    std::size_t getSize(void);
    std::string getRoot(void);
    std::string getIP(void);
    int getPort(void);
    bool getDirectoyListing(void);
    std::vector<std::string> getAcceptedMethods(void);
    std::pair<int, std::string> getRedir(void);
    std::map<std::string, std::string> getCgi(void);
};

#endif
