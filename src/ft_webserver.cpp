#include "../includes/ft_webserver.hpp"

int main(int argc, char **argv)
{
    std::string file;
    if (argc == 1)
        file = "conf/configExample.conf";
    else
        file = argv[1];

    try {
        Server s(file);
        s.startServer();
    }
    catch (std::exception &e) {
        std::cout << "exception caught : " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
