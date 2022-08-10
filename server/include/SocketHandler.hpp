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

		std::map<int, int> _serverMap;
		int _kq;
		int _numEvents;
		// char _buffer[1024]; //temp
		struct kevent _ev; // temp
		std::string _buffer; //temp
		int _fd; //temp


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
		void acceptConnection(int i);
		bool addSocket(int fd);
		bool readFromClient(int i);
		bool writeToClient(int i);
		bool removeClient(int fd, bool force = false);
		void removeInactiveClients();

	// Getter
		int getNumEvents() const;
		// const char *getBuffer() const;
		std::string getBuffer() const;
		int getFD(int i) const;

	// Setter
		void setWriteable(int i);
		void setReadable(int i);

};
#endif // SOCKETHANDLER_HPP
