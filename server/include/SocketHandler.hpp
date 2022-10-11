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

// classes
class SocketHandler
{
	private:
		SocketHandler();
		SocketHandler(const SocketHandler &src);
		SocketHandler &operator=(const SocketHandler &src);

	// Private Variables
		std::set<int> _ports;
		std::map<std::string, ConfigStruct> _cluster;
		std::set<int> _keepalive;
		std::vector<ClientStruct> _clients;
		std::vector<int> _serverFds;

		struct kevent _evList[MAX_EVENTS];
		std::vector<struct kevent> _eventsChanges;

		std::map<int, int> _serverMap;
		int _kq;

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
		~SocketHandler();

	// Overloaded Operators

	// Public Methods
		int getEvents();
		bool acceptConnection(int i);
		void addSocket(int fd);
		bool readFromClient(int i);
		bool writeToClient(int i);

		bool removeClient(int fd, bool force = false);
		int removeInactiveClients();
		std::string createTimeoutResponse();

		void addKeepAlive(int clientFd);
		void removeKeepAlive(int clientFd);
		bool isKeepAlive(int clientFd);

	// Getter
		int getFD(int i) const;
		int getPort(int i);

	// Setter
		void setTimeout(int clientFd);
		void setWriteable(int i);
		void setEvent(int ident, int flags, int filter);

	// Other
	void setNonBlocking(int fd);
	void setNoSigpipe(int fd);

};

#endif // SOCKETHANDLER_HPP
