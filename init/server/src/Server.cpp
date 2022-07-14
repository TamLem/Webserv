#include "Server.hpp"
#include "Config.hpp"

static void handle_signal(int sig)
{
	if (sig == SIGINT)
	{
		std::cerr << BLUE << "SIGINT detected, terminating server now" << RESET << std::endl;
		keep_running = 0;
	}
	// else if (sig == SIGPIPE)
	// {
	// 	std::cerr << RED << "SIGPIPE detected, will crash now" << RESET << std::endl;
	// 	keep_running = 0;
	// }
}

// check if signal is forbidden!!!!!!!!!!!!!!!!!
void	handle_signals(void)
{
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	// signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, handle_signal);
	// signal(SIGPIPE, handle_signal);
}

void parseArgv(int argc, char **argv)
{
	if (argc <= 1 || argc > 2)
	{
		std::cerr << RED << "Please only use webserv with config file as follows:" << std::endl << BLUE << "./webserv <config_filename>.conf" << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	std::string sArgv = argv[1];
	std::string ending = ".conf";
	if ((argv[1] + sArgv.find_last_of(".")) != ending)
	{
		std::cerr << RED << "Please only use webserv with config file as follows:" << std::endl << BLUE << "./webserv <config_filename>.conf" << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
	handle_signals();
	parseArgv(argc, argv);
	Config config;
	try
	{
		config.start(argv[1]);
	}
	catch (std::exception &e)
	{
			std::cerr << RED << e.what() << RESET << std::endl;
			return (EXIT_FAILURE);
	}
	Server test(8080); // somehow pass the listen ports to the server ??
	test.cluster = config.getCluster();
	test.run();
	return (0);
}
