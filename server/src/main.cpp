#include "Server.hpp"
#include "Config.hpp"

#include <stdlib.h>

static std::string parseArgv(int argc, char **argv)
{
	std::string defaultConfPath = DEFAULT_CONFIG; // !!!!!!!!!!!!!!!! test.conf is now default
	if (argc == 1)
	{
		return (defaultConfPath);
	}
	else if (argc > 2)
	{
		std::cerr << RED << "Please use webserv with config file only as follows:" << std::endl << BLUE << "./webserv <config_filename.conf>" << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	std::string sArgv = argv[1];
	std::string ending = ".conf";
	if ((argv[1] + sArgv.find_last_of(".")) != ending)
	{
		std::cerr << RED << "Please use webserv with config file only as follows:" << std::endl << BLUE << "./webserv <config_filename.conf>" << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	return (sArgv);
}

void my_leaks()
{
	system("leaks webserv");
}

void handle_signal(int sig)
{
	if (sig == SIGINT || sig == SIGQUIT)
	{
		std::cerr << BLUE << "SIGINT detected, terminating server now" << RESET << std::endl;
		keep_running = 0;
	}
	else if (sig == SIGPIPE)
	{
		std::cerr << RED << "SIGPIPE detected, will end now" << RESET << std::endl;
		keep_running = 0;
	}
}

void handle_signals(void)
{
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	// signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, handle_signal);
	// signal(SIGPIPE, handle_signal);
}

int main(int argc, char **argv)
{
	// atexit(my_leaks); // use this to check for leaks
	Config *config = new Config();
	try
	{
		config->start(parseArgv(argc, argv));
	}
	catch (std::exception &e)
	{
			std::cerr << RED << e.what() << RESET << std::endl;
			delete config;
			return (EXIT_FAILURE);
	}
	Server *test = new Server(config);

	#ifdef SHOW_LOG_2
		config->printCluster();
	#endif
	test->runEventLoop();
	delete config;
	delete test;
	config = NULL;
	test = NULL;
	// system("leaks webserv"); // use this to check for leaks
	return (0);
}
