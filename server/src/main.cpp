#include "Server.hpp"
#include "Config.hpp"

#include <stdlib.h>

static std::string parseArgv(int argc, char **argv)
{
	std::string defaultConfPath = "server/config/test.conf"; // !!!!!!!!!!!!!!!! test.conf is now default
	if (argc == 1)
	{
		return (defaultConfPath);
	}
	else if (argc > 2)
	{
		std::cerr << RED << "Please only use webserv with config file as follows:" << std::endl << BLUE << "./webserv <config_filename.conf>" << RESET << std::endl;
		exit(EXIT_FAILURE); // maybe change to return or throw
	}
	std::string sArgv = argv[1];
	std::string ending = ".conf";
	if ((argv[1] + sArgv.find_last_of(".")) != ending)
	{
		std::cerr << RED << "Please only use webserv with config file as follows:" << std::endl << BLUE << "./webserv <config_filename.conf>" << RESET << std::endl;
		exit(EXIT_FAILURE); // maybe change to return or throw
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
		#ifdef __APPLE__
			std::cerr << BLUE << "SIGINT detected, terminating server now" << RESET << std::endl;
			keep_running = 0;
		#else
			throw std::runtime_error("SIGINT detected, terminating server now");
		#endif
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
	#ifndef __APPLE__
		handle_signals();
	#endif
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
	#ifdef __APPLE__
		test->runEventLoop();
	#else
		try
		{
			test->runEventLoop();
		}
		catch (std::exception &e)
		{
			std::cerr << BLUE << e.what() << RESET << std::endl;
			delete config;
			delete test;
			config = NULL;
			test = NULL;
			// system("leaks webserv"); // use this to check for leaks
			return (0);
		}
	#endif
	delete config;
	delete test;
	config = NULL;
	test = NULL;
	// system("leaks webserv"); // use this to check for leaks
	return (0);
}
