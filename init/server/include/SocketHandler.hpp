// Header-protection
#ifndef SOCKETHANDLER_HPP
#define SOCKETHANDLER_HPP
#pragma once

// Includes
#include <string>
#include <iostream>
#include <set>
#include <map>

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include "Base.hpp"
#include "Config.hpp"

#define MAX_EVENTS 128

// struct ServerStruct
// {
// 	unsigned short fd;
// 	struct sockaddr_in serv_addr;
// };

struct ClientStruct
{
	int fd;
	struct sockaddr_in addr;
};

// classes
class SocketHandler
{
	private:
	// Private Variables
		Config *_config;
		std::set<int> _ports;
		std::vector<ClientStruct> _clients;
		std::vector<int> _serverFds; // maybe not needed
		struct kevent _evList[MAX_EVENTS];
		std::map<int, int> _serverMap;
		int _kq;

	// Private Members
		void _initPorts(); // read from ConfigStruct into _ports
		void _initMainSockets();
		void _listenMainSockets();
		void _initEventLoop();
		void _getEvents();
		void _acceptConnection();
		void _addClient();
		void _removeClient();
		void _getClient();

	public:
	// Constructors
		SocketHandler(Config *config);
		SocketHandler(const SocketHandler &src);

	// Deconstructors
		~SocketHandler(); // have a loop that closes all used fd's stored in the ServerStruct

	// Overloaded Operators
		SocketHandler &operator=(const SocketHandler &src);

	// Public Methods

	// Getter

	// Setter

};
#endif // SOCKETHANDLER_HPP
