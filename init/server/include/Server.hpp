#ifndef SERVER_HPP
#define SERVER_HPP

/* system includes */
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <sys/event.h>
#include <map>
#include <set>
#include <csignal> // check if forbidden !!!!!!!!!!

/* our includes */
#include "Config.hpp"
#include "Response.hpp"
#include "Request.hpp"
#include "SingleServerConfig.hpp"
#include "Base.hpp"

#ifdef __APPLE__
	#include "SocketHandler.hpp"
#else
	#include "LinuxSocketHandler.hpp"
#endif

// Forbidden includes maybe??????
#include <errno.h>

struct client
{
	int fd;
	struct sockaddr_in addr;
};

void cgi_handle(Request& request, std::string buf, int fd);
#define MAX_EVENTS 128

static volatile int keep_running = 1;

class Server
{
	public:
		Server(Config* config);
		~Server(void);

		// void stop(void);

		// int get_client(int fd);

		// int add_client(int fd, struct sockaddr_in addr);

		// int remove_client(int fd);

		void runEventLoop(void);

		// void run(void);

		void handleRequest(/*const std::string&, */int);
	private:
		Config *_config;
		SocketHandler *_socketHandler;
		// size_t _port;
		// std::set<unsigned short> _ports; // moved to socket handler
		// size_t _server_fd;
		std::vector <client> _clients;
		Response _response;
		std::string _requestHead;
		ConfigStruct _currentConfig;

		Server();
		static void handle_signal(int sig);
		void handle_signals(void);
		bool _crlftwoFound();
		bool _isPrintableAscii(char c);
		void _readRequestHead(int fd);

		void applyCurrentConfig(const Request&);
		void matchLocation(Request&);
		std::string percentDecoding(const std::string&);
		void handleGET(const Request&);
		void handlePOST(const Request&);
		void handleERROR(const std::string&);

	class InvalidHex : public std::exception
	{
		const char* what() const throw();
	};
	// Exceptions
		class InternatServerErrorException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class BadRequestException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class FirstLineTooLongException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};
};

void cgi_handle(Request& request, std::string buf, int fd);

#endif