#include "../includes/ft_webserver.hpp"

int main(int argc, char **argv)
{
	std::string file;
	if (argc == 1)
		file = "conf/configExample.conf";
	else
		file = argv[1];

	Server s(file);
	s.startServer();
	return 0;
}
