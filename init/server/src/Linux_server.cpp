#include <iostream>
#include <ios>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include "Request.hpp"
#include "Response.hpp"
#include "Base.hpp"

#include "Config.hpp"
#include "Cgi/Cgi.hpp"

// Forbidden includes
#include <errno.h>

void cgi_handle(Request& request, std::string buf, int fd);

// this was the first try of a socket connection, now is only used to have a working linux compatible simple server

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

static Config *_config;
static Response _response;

void handleGET(const std::string& status, int fd, const std::string& uri)
{
	_response.init(status, fd, uri);
	_response.createBody();
	_response.createHeaderFields();
	_response.sendResponse();
}

void handlePOST(const std::string& status, int fd, const Request& newRequest)
{
	std::ofstream outFile;
	outFile.open(("./uploads/" + newRequest.getBody()).c_str());
	if (outFile.is_open() == false)
		throw std::exception();
	outFile << newRequest.getBody() << "'s content. Server: " << _config->getConfigStruct("weebserv").serverName;
	outFile.close();
	_response.init(status, fd, "./pages/post_test.html");
	_response.createBody();
	_response.createHeaderFields();
	_response.sendResponse();
}

void handleERROR(const std::string& status, int fd)
{
	_response.init(status, fd, ""); //AE make overload instead of passing ""
	_response.createErrorBody();
	_response.createHeaderFields();
	_response.sendResponse();
}

void handleRequest(const std::string& buffer, int fd)
{
	try
	{
		Request newRequest(buffer);
		if (buffer.find("/cgi/") != std::string::npos)
			cgi_handle(newRequest, buffer, fd);
		else if (newRequest.getMethod() == "POST")
			handlePOST("200", fd, newRequest);
		else
			handleGET("200", fd, newRequest.getUri());
	}
	catch (std::exception& exception)
	{
		std::string code = exception.what();
		if (_response.getMessageMap().count(code) != 1)
			code = "500";
		handleERROR(code, fd);
	}
	// _response.sendResponse();
}

void cgi_handle(Request& request, std::string buf, int fd)
{
	Cgi newCgi(request);

	newCgi.printEnv();
	newCgi.cgi_response(buf, fd);
}

static bool keep_running;

int main(int argc, char **argv)
{
	keep_running = true;
	parseArgv(argc, argv);
	Config *config = new Config();
	_config = config;
	try
	{
		config->start(argv[1]);
	}
	catch (std::exception &e)
	{
			std::cerr << RED << e.what() << RESET << std::endl;
			delete config;
			return (EXIT_FAILURE);
	}
	int sockfd;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	const int port = 8080;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cout << "Error creating socket" << std::endl;
		return 1;
	}

// Set socket reusable from Time-Wait state
	int val = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, 4);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(serv_addr.sin_zero, '0', sizeof(serv_addr.sin_zero));

	if((bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
	{
		std::cout << RED << "Error binding socket" << std::endl;
		perror("bind");
		std::cerr << RESET;
		return 1;
	}
	else
		std::cout << "Server listening on port " << port << std::endl;

	if (listen(sockfd, 5) < 0)
	{
		std::cout << "Error listening on socket" << std::endl;
		return 1;
	}

	int new_sockfd;
	while(keep_running)
	{
		socklen_t clilen = sizeof(cli_addr);
		if ((new_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) < 0)
		{
			std::cout << "Error accepting connection" << std::endl;
			return 1;
		}
		std::cout << "Connection accepted" << std::endl;
		char buffer[10000] = {0};
		if (read(new_sockfd, buffer, sizeof(buffer)) < 0)
		{
			std::cout << "Error reading from socket" << std::endl;
			return 1;
		}
		std::cout << "Message received: " << buffer << std::endl;
		handleRequest(buffer, new_sockfd);
		keep_running = false;
		close(new_sockfd);
	}
	close(sockfd);
	config->printCluster();
	delete config;
	config = NULL;
	return 0;
}
