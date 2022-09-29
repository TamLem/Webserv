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
#include <map>
#include <set>
#include <csignal>

/* our includes */
#include "Config.hpp"
#include "Response.hpp"
#include "Request.hpp"
#include "SingleServerConfig.hpp"
#include "Base.hpp"

#define UPLOAD_DIR "./server/data/uploads/"

#ifdef __APPLE__
	#include <sys/event.h>
	#include "SocketHandler.hpp"
#else
	#include "LinuxSocketHandler.hpp"
#endif

// Forbidden includes maybe??????
#include <errno.h>

// struct client
// {
// 	int fd;
// 	struct sockaddr_in addr;
// };

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

		void handleRequest(int clientFd);
	private:
	// defines only to not have undefined behaviour
		Server(const Server&);
		Server& operator=(const Server&);
		Server(void);
	// private Members
		Config *_config;
		SocketHandler *_socketHandler;

		// std::vector <ClientStruct> _clients;
		Response _response;// needs to go away
		std::string _requestHead;
		ConfigStruct _currentConfig;
		std::string _currentLocationKey;
		bool loopDetected;
		std::vector<int> _cgiSockets;

	// private Methods
		static void handle_signal(int sig);
		void handle_signals(void);
		bool _crlftwoFound();
		bool _isPrintableAscii(char c);
		void _readRequestHead(int fd);

		void applyCurrentConfig(const Request&);
		void removeClientTraces(int clientFd);
		void matchLocation(Request&);
		int routeFile(Request&, std::map<std::string, LocationStruct>::const_iterator, const std::string&);
		void routeDir(Request&, std::map<std::string, LocationStruct>::const_iterator, const std::string&, int&);
		void routeDefault(Request&);
		std::string percentDecoding(const std::string&);
		void checkLocationMethod(const Request& request) const;
		void handleGET(const Request&);
		void handlePOST(int clientFd, const Request&); // maybe passs fd to this function
		void handleERROR(const std::string&);
		void handleDELETE(const Request& request);
		void _handleResponse(int i);

		bool _isCgiRequest(std::string requestHead);
		void cgi_handle(Request& request, int fd, ConfigStruct configStruct, FILE *tempFile);

	public:

	// Exceptions
		class ClientDisconnect: public  std::exception {
			public:
				virtual const char* what() const throw();
		};
		class InternalServerErrorException : public std::exception
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

		class InvalidHex : public std::exception
		{
			const char* what() const throw();
		};

		class MethodNotAllowed : public std::exception
		{
			const char* what() const throw();
		};

		class LengthRequiredException : public std::exception
		{
			const char* what() const throw();
		};

		class ContentTooLargeException : public std::exception
		{
			const char* what() const throw();
		};
};

// std::string percentDecodingFix(std::string target);
std::string staticReplaceInString(std::string str, std::string tofind, std::string toreplace);

#endif