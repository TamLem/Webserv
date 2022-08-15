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
#include <fcntl.h>

#include "Base.hpp"
#include "Config.hpp"

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
	// defines only to not have undefined behaviour
		SocketHandler();
		SocketHandler(const SocketHandler &src);
		SocketHandler &operator=(const SocketHandler &src);

	// Private Variables
		// Config *_config; // maybe not needed
		std::map<std::string, ConfigStruct> _cluster;

		std::set<int> _ports;
		std::vector<ClientStruct> _clients;
		std::vector<int> _serverFds; // maybe not needed

		struct kevent _evList[MAX_EVENTS];
		std::vector<struct kevent> _eventsChanges;

		std::map<int, int> _serverMap;
		int _kq;
		// char _buffer[1024]; //temp
		struct kevent _ev; // temp


	// Private Members
		void _initPorts(); // read from ConfigStruct into _ports
		void _initMainSockets();
		void _listenMainSockets();
		void _initEventLoop();
		int _addClient(int fd, struct sockaddr_in addr);
		int _getClient(int fd);
		bool _isPrintableAscii(char c);

	public:
	// Constructors
		SocketHandler(Config *config);

	// Deconstructors
		~SocketHandler(); // have a loop that closes all used fd's stored in the ServerStruct

	// Overloaded Operators

	// Public Methods
		int getEvents();
		bool acceptConnection(int i);
		bool addSocket(int fd);
		bool readFromClient(int i);
		bool writeToClient(int i);
		bool removeClient(int fd, bool force = false);
		void removeInactiveClients();

	// Getter
		int getFD(int i) const;

	// Setter
		void setWriteable(int i);
		void setEvent(int ident, int flags, int filter);

	// Other
	void setNonBlocking(int fd);
	void setNoSigpipe(int fd);



};


#endif // SOCKETHANDLER_HPP
