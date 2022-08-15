#include "SocketHandler.hpp"

// Private Members
void SocketHandler::_initPorts()
{
	// inits this->_ports
	std::map<std::string, ConfigStruct>::const_iterator confIt = this->_cluster.begin();
	std::map<std::string, ConfigStruct>::const_iterator confEnd = this->_cluster.end();
	for (; confIt != confEnd; ++confIt)
	{
		std::map<std::string, unsigned short>::const_iterator listenIt = confIt->second.listen.begin();
		std::map<std::string, unsigned short>::const_iterator listenEnd = confIt->second.listen.end();
		for (; listenIt != listenEnd; ++listenIt)
		{
			this->_ports.insert(listenIt->second);
		}
	}
}

void SocketHandler::_initMainSockets()
{
	for (std::set<int>::iterator portsIt = this->_ports.begin(); portsIt != this->_ports.end(); ++portsIt)
	{
		int tempFD;
		if ((tempFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			std::cerr << RED << "Error creating socket" << std::endl;
			perror(NULL);
			std::cerr << RESET;
			return; // throw error
		}
		else
		{
			this->_serverFds.push_back(tempFD);
			this->_serverMap.insert(std::make_pair<int, int>(tempFD, *portsIt));
		}

		// Set socket reusable from Time-Wait state
		int val = 1;
		setsockopt(tempFD, SOL_SOCKET, SO_REUSEADDR, &val, 4);

		// initialize server address struct
		struct sockaddr_in servAddr;
		memset(&servAddr, '0', sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		servAddr.sin_port = htons(*portsIt);
		servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		// bind socket to address
		if((bind(tempFD, (struct sockaddr *)&servAddr, sizeof(servAddr))) < 0)
		{
			std::cerr << RED << "Error binding socket: " << tempFD << std::endl;
			perror(NULL);
			std::cerr << RESET;
			exit(EXIT_FAILURE); // maybe change to throw
		}
		std::cout << GREEN << "succeccfully bound socket " << *portsIt << RESET << std::endl;
	}

}

void SocketHandler::_listenMainSockets()
{
	for (std::map<int, int>::const_iterator it = this->_serverMap.begin(); it != this->_serverMap.end(); ++it)
	{
		if (listen(it->first, 5))
		{
			std::cerr << RED << "Error listening" << std::endl;
			perror(NULL);
			std::cerr << RESET;
			return; // throw error
		}
		else
			std::cout << "Listening on port " << it->second << " with fd: " << it->first << std::endl;
	}
}

void SocketHandler::_initEventLoop()
{
	this->_kq = kqueue();
	if (this->_kq == -1)
	{
		std::cerr << RED << "Error creating kqueue" << std::endl;
		perror(NULL);
		std::cerr << RESET;
		return; // throw error
	}
	for (std::vector<int>::const_iterator it = this->_serverFds.begin(); it != this->_serverFds.end(); ++it)
	{
		this->setEvent(*it, EV_ADD, EVFILT_READ);
	}
}

bool SocketHandler::acceptConnection(int i)
{
	if (this->_serverMap.count(this->_evList[i].ident) == 1)
	{
		struct sockaddr_storage addr; // temp
		socklen_t addrlen = sizeof(addr); //temp
		int fd = accept(this->_evList[i].ident, (struct sockaddr *)&addr, &addrlen);
		if (fd < 0)
		{
			std::cerr << RED << "Error accepting connection" << std::endl;
			perror(NULL);
			std::cerr << RESET;
			exit(1);
			return true; // throw exception here
		}
		else if (this->addSocket(fd))
		{
			#ifdef SHOW_LOG
				std::cout << GREEN << "New connection on socket " << fd << RESET << std::endl;
			#endif
			this->_addClient(fd, *(struct sockaddr_in *)&addr);
		}
		else
		{
			#ifdef SHOW_LOG
				std::cout << RED << "Error adding socket " << fd << RESET << std::endl;
			#endif
			close(fd);
		}
		return true;
	}
	return false;
}

void SocketHandler::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void SocketHandler::setNoSigpipe(int fd)
{
	int val = 1;
	setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &val, 4);
}

bool SocketHandler::addSocket(int fd)
{
	this->setNonBlocking(fd);
	this->setNoSigpipe(fd);

	this->setEvent(fd, EV_ADD, EVFILT_READ);
	return true;
}

int SocketHandler::getEvents()
{
	struct timespec timeout;

	timeout.tv_sec = 0;
	timeout.tv_nsec = 100;
	#ifdef SHOW_LOG_2
		std::cout << "num clients: " << _clients.size() << std::endl;
	#endif
	int numEvents = kevent(this->_kq, this->_eventsChanges.data(), this->_eventsChanges.size(), this->_evList, MAX_EVENTS, NULL);
	this->_eventsChanges.clear();
	if (numEvents == -1)
	{
		std::cerr << RED << "Error getting events" << std::endl;
		perror(NULL);
		std::cerr << RESET;
		return -1;
	}
	return numEvents;
}

int SocketHandler::_addClient(int fd, struct sockaddr_in addr)
{
	struct ClientStruct c;
	c.fd = fd;
	c.addr = addr;
	this->_clients.push_back(c);
	return (this->_clients.size() - 1);
}

bool SocketHandler::removeClient(int i, bool force)
{
	if ((this->_evList[i].flags == EV_EOF )  || (this->_evList[i].flags == EV_ERROR )/* || (this->_evList[i].flags & EV_CLEAR) */ || force)
	{
		#ifdef SHOW_LOG_2
			std::cout << RED << (force ? "Kicking client " : "Removing client ") <<  "fd: " << RESET << this->_evList[i].ident << std::endl;
		#endif
		close(this->_evList[i].ident);
		int index = this->_getClient(this->_evList[i].ident);
		if (index != -1)
		{

			// this->setEvent(this->_evList[i].ident, EV_DELETE, EVFILT_WRITE); //event will be removed automatically when the fd is closed
			this->_clients.erase(this->_clients.begin() + index);
			#ifdef SHOW_LOG
				std::cout << RED << "Client " << this->_evList[i].ident << " disconnected" << RESET << std::endl;
			#endif
			return (true);
		}
		else
		{
			std::cout << "error getting client on fd: " << this->_evList[i].ident << std::endl;
			exit(112);
		}
	}
	return (false);
}

bool SocketHandler::readFromClient(int i)
{
	if (this->_serverMap.count(this->_evList[i].ident) == 0 && this->_evList[i].filter == EVFILT_READ)
	{
		int fd = this->_evList[i].ident;
		int status = this->_getClient(fd);
		if (status == -1)
		{
			close(fd);
			std::cerr << RED << "read Error getting client for fd: " << fd << std::endl;
			perror(NULL); // check if illegal
			std::cerr << RESET;
			return (false); // throw exception
		}
		return (true);
	}
	else
		return (false);
}

bool SocketHandler::writeToClient(int i)
{
	if (this->_evList[i].flags == EV_ERROR)
		return (false);
	if (this->_serverMap.count(this->_evList[i].ident) == 0 && this->_evList[i].filter == EVFILT_WRITE)
	{
		int fd = this->_evList[i].ident;
		int status = this->_getClient(fd);
		if (status == -1)
		{
			close(fd); 
			std::cerr << RED << "write Error getting client for fd: " << fd << std::endl;
			perror(NULL); // check if illegal
			std::cerr << RESET;
			return (false); // throw exception
		}
		return (true);
	}
	else
		return (false);
}

int SocketHandler::_getClient(int fd)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i].fd == fd)
			return (i);
	}
	return (-1);
}

// Constructors
SocketHandler::SocketHandler(Config *config)
{
	#ifdef SHOW_CONSTRUCTION
		std::cout << GREEN << "SocketHandler Default Constructor called for " << this << RESET << std::endl;
	#endif
	this->_cluster = config->getCluster();
	this->_initPorts();
	this->_initMainSockets();
	this->_listenMainSockets();
	this->_initEventLoop();
}

void SocketHandler::removeInactiveClients()
{
	#ifdef SHOW_LOG
		std::cout << RED << "Clear all connections" << RESET << std::endl;
	#endif
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		close(this->_clients[i].fd);
	}
	this->_clients.clear();
}

// Deconstructors
SocketHandler::~SocketHandler()
{
	for (std::vector<int>::const_iterator it = this->_serverFds.begin(); it != this->_serverFds.end(); ++it)
		close(*it);
	close(this->_kq);
	#ifdef SHOW_CONSTRUCTION
		std::cout << RED << "SocketHandler Deconstructor called for " << this << RESET << std::endl;
	#endif
}

// Overloaded Operators

// Public Methods

// Getter
int SocketHandler::getFD(int i) const
{
	return (this->_evList[i].ident);
}

// Setter
void SocketHandler::setWriteable(int i)
{
	int fd = this->_evList[i].ident;
	// this->setEvent(fd, EV_DELETE, EVFILT_READ);
	this->setEvent(fd, EV_ADD, EVFILT_WRITE);

	this->setNoSigpipe(fd);
	this->setNonBlocking(fd);
}

void SocketHandler::setEvent(int ident, int flags, int filter)
{
	struct kevent ev;

	EV_SET(&ev, ident, filter, flags, 0, 0, NULL);
	this->_eventsChanges.push_back(ev);
}