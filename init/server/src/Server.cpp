#include "Server.hpp"
#include "Config.hpp"

void Server::handleGET(int status, int fd, const std::string& uri)
{
	Response.init(status, fd, uri);
	Response.createBody();
	Response.createHeaderFields();
	Response.sendResponse();
}

void Server::handlePOST(int status, int fd, const Request& newRequest)
{
	std::ofstream outFile;
	outFile.open("./uploads/" + newRequest.getBody());
	if (outFile.is_open() == false)
		throw std::exception();
	outFile << newRequest.getBody() << "'s content.";
	outFile.close();
	Response.init(status, fd, "./pages/post_test.html");
	Response.createBody();
	Response.createHeaderFields();
	Response.sendResponse();
}

void Server::handleERROR(int status, int fd)
{
	Response.init(status, fd, ""); //AE make overload instead of passing ""
	Response.createErrorBody();
	Response.createHeaderFields();
	Response.sendResponse();
}

void Server::handle_static_request(const std::string& buffer, int fd)
{
	try
	{
		Request newRequest(buffer);
		if (newRequest.getMethod() == "POST")
			handlePOST(200, fd, newRequest);
		else
			handleGET(200, fd, newRequest.getUri());
	}
	catch (Request::InvalidMethod& e)
	{
		handleERROR(501, fd);
	}
	catch (Request::InvalidProtocol& e)
	{
		handleERROR(505, fd);
	}
	catch (Response::ERROR_404& e)
	{
		handleERROR(404, fd);
	}
	catch (Message::BadRequest& e)
	{
		handleERROR(400, fd);
	}
	catch (std::exception& e)
	{
		handleERROR(500, fd);
	}
}

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
