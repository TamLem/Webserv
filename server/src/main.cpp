#include "Server.hpp"
#include "Config.hpp"

#include <stdlib.h>

// #include <ulimit.h>

std::string parseArgv(int argc, char **argv) // include this into some object, maybe config would be appropriate, since it does do the parsing
{
	std::string defaultConfPath = "config/www.conf";
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

int main(int argc, char **argv)
{
	// ulimit(UL_SETFSIZE, 64);
	// atexit(my_leaks);
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
	// system("leaks webserv");
	return (0);
}
