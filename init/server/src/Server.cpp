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

void parseArgv(int argc, char **argv) // maybe change to static void function or include it into some object
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
	Config *config = new Config();
	try
	{
		config->start(argv[1]);
	}
	catch (std::exception &e)
	{
			std::cerr << RED << "Exception caught in main function: " << e.what() << RESET << std::endl;
			return (EXIT_FAILURE);
	}
	Server *test = new Server(8080); // somehow pass the listen ports to the server ??
	test->cluster = config->getCluster();
	delete config;
	// std::cout << BLUE << test->cluster.size() << " elements found inside map" << RESET << std::endl;
	// test if the data inside the cluster is accessable
	std::string firstName = "weebserv";
	std::string secondName = "anotherone";

	SingleServerConfig first = test->cluster[firstName];
	SingleServerConfig second = test->cluster[secondName];

	if (test->cluster.count("weebserv") == 1)
		std::cout << "server weebserv found in cluster with address " << &test->cluster[firstName] << std::endl;
	else
		return (EXIT_FAILURE);
	if (test->cluster.count("anotherone") == 1)
		std::cout << "server anotherone found in cluster with address " << &test->cluster[secondName] << std::endl;
	else
		return (EXIT_FAILURE);
	// std::cout << RED << first.getServerName() << "<->" << second.getServerName() << RESET << std::endl;
	std::cout << first << std::endl;
	std::cout << second << std::endl;
	// system("leaks webserv");
	// test.run();
	delete test;
	return (0);
}
