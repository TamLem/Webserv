#include "Server.hpp"
#include "Config.hpp"

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
	parseArgv(argc, argv);
	Config *config = new Config();
	try
	{
		config->start(argv[1]);
	}
	catch (std::exception &e)
	{
			std::cerr << RED << "Exception caught in main function: " << e.what() << RESET << std::endl;
			delete config;
			return (EXIT_FAILURE);
	}
	Server *test = new Server(8080/*, &config*/); // somehow pass the listen ports to the server ??
	// std::cout << BLUE << test->cluster.size() << " elements found inside map" << RESET << std::endl;
	// test if the data inside the cluster is accessable
	std::string firstName = "weebserv";
	std::string secondName = "anotherone";

	// SingleServerConfig first = test->cluster[firstName];
	// SingleServerConfig second = test->cluster[secondName];
	std::cout << "### attempting to print contents of the configStructs" << std::endl;
	config->applyConfig(firstName);
	std::cout << config << std::endl;
	config->applyConfig(secondName);
	std::cout << config << std::endl;
	// std::cout << RED << first.getServerName() << "<->" << second.getServerName() << RESET << std::endl;
	// system("leaks webserv");
	test->run();
	delete config;
	delete test;
	config = NULL;
	test = NULL;
	// system("leaks webserv");
	return (0);
}
