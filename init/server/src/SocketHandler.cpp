#include "SocketHandler.hpp"

// Private Members
void SocketHandler::_initPorts()
{
	// inits this->_ports
	// std::map<std::string, ConfigStruct>::const_iterator confIt = this->_cluster.begin();
	// std::map<std::string, ConfigStruct>::const_iterator confEnd = this->_cluster.end();
	// for (; confIt != confEnd; ++confIt)
	// {

	// }
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
		setsockopt(tempFD, SOL_SOCKET, SO_REUSEADDR, &val, 4); // is SO_NOSIGPIPE needed here ???????

		// initialize server address struct
		struct sockaddr_in servAddr;
		memset(&servAddr, '0', sizeof(servAddr)); // is memset allowed? !!!!!!!!!!!!!!!!!!!!
		servAddr.sin_family = AF_INET;
		servAddr.sin_port = htons(*portsIt);
		servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		// bind socket to address
		if((bind(tempFD, (struct sockaddr *)&servAddr, sizeof(servAddr))) < 0)
		{
			std::cerr << RED << "Error binding socket" << std::endl;
			perror(NULL);
			std::cerr << RESET;
			exit(EXIT_FAILURE); // maybe change to throw
		}
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
			std::cout << "Listening on port " << it->second << std::endl;
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
		struct kevent ev;
		EV_SET(&ev, *it, EVFILT_READ, EV_ADD, 0, 0, NULL);
		if (kevent(this->_kq, &ev, 1, NULL, 0, NULL) == -1)
		{
			std::cerr << RED << "Error adding server socket to kqueue" << std::endl;
			perror(NULL);
			std::cerr << RESET;
			return; // throw error
		}
	}
}

void SocketHandler::getEvents()
{
	this->_numEvents = kevent(this->_kq, NULL, 0, this->_evList, MAX_EVENTS, NULL);
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
			return (false); // throw exception here
		}
		else
		{
			#ifdef SHOW_LOG
				std::cout << GREEN << "New connection on socket " << fd << RESET << std::endl;
			#endif
			int set = 1;
			struct kevent ev;
			setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int)); // set socket to not SIGPIPE
			this->_addClient(fd, *(struct sockaddr_in *)&addr);
			EV_SET(&ev, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
			kevent(this->_kq, &ev, 1, NULL, 0, NULL);
			this->_fd = fd;
			return (true);
		}
	}
	else
		return (false);
}

int SocketHandler::_addClient(int fd, struct sockaddr_in addr)
{
	struct ClientStruct c;
	c.fd = fd;
	c.addr = addr;
	this->_clients.push_back(c);
	return (this->_clients.size() - 1); // is this return value ever used??????
}

int SocketHandler::removeClient(int i)
{
	if (this->_evList[i].flags & EV_EOF)
	{
		int i = this->_getClient(this->_fd);
		if (i == -1)
			return (-1);
		this->_clients.erase(this->_clients.begin() + i);
		return (0);
	}
}

bool SocketHandler::readFromClient(int i)
{
	if (this->_evList[i].flags & EVFILT_READ)
	{
		this->_fd = this->_evList[i].ident;
		int i = this->_getClient(this->_fd);
		if (i == -1)
		{
			std::cerr << RED << "Error getting client" << std::endl;
			perror(NULL);
			std::cerr << RESET;
			return (false); // throw exception
		}
		// return (true);


	// this should happen inside request object
		char buf[1024];
		int n = 0;
		this->_buffer.clear();
		while ((n = read(this->_fd, buf, 1024)) > 0)
		{
			if (this->_isPrintableAscii(buf) == true)
				this->_buffer.append(buf);
		}
		if (n < 0)
		{
			std::cerr << RED << "Error reading from client" << std::endl;
			perror(NULL);
			std::cerr << RESET;
			return (false); // throw exception
		}
		#ifdef SHOW_LOG
			std::cout << YELLOW << "Received->" << RESET << buf << YELLOW << "<-Received" << RESET << std::endl;
		#endif
		return (true);
	}
	else
		return (false);
}

bool SocketHandler::_isPrintableAscii(char *str)
{
	int i = 0;
	while (str && str[i])
	{
		if (str[i] < 32 || str[i] > 126)
			break ;
		++i;
	}
	if (str && str[i] == NULL)
		return (true);
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
SocketHandler::SocketHandler(Config *config): _cluster(config->getCluster())
{
	#ifdef SHOW_LOG
		std::cout << GREEN << "SocketHandler Default Constructor called for " << this << RESET << std::endl;
	#endif
	this->_initPorts();
	this->_initMainSockets();
	this->_initEventLoop();
}

// Deconstructors
SocketHandler::~SocketHandler()
{
	for (std::vector<int>::const_iterator it = this->_serverFds.begin(); it != this->_serverFds.end(); ++it)
		close(*it);
	#ifdef SHOW_LOG
		std::cout << RED << "SocketHandler Deconstructor called for " << this << RESET << std::endl;
	#endif
}

// Overloaded Operators

// Public Methods

// Getter
int SocketHandler::getNumEvents() const
{
	return (this->_numEvents);
}

std::string SocketHandler::getBuffer() const
{
	// check buffer for non ascii maybe? or do that in the read loop
	return (this->_buffer);
}

// Setter

