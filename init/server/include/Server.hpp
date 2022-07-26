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

		void stop(void);

		int get_client(int fd);

		int add_client(int fd, struct sockaddr_in addr);

		int remove_client(int fd);

		void run_event_loop(int kq);

		void run(void);

		void handleRequest(const std::string&, int);
		void handleGET(const std::string&, int, const std::string&);
		void handlePOST(const std::string&, int, const Request&);
		void handleERROR(const std::string&, int);
	private:
		size_t _port;
		// std::set<unsigned short> _ports; // moved to socket handler
		size_t _server_fd;
		std::vector <client> _clients;
		Response _response;
		SocketHandler _socketHandler;
		Config* _config; // think about const

		Server();
		static void handle_signal(int sig);
		void handle_signals(void);
		void _initPorts();
};

void cgi_handle(Request& request, std::string buf, int fd);

#endif