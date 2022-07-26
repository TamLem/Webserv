#include "SocketHandler.hpp"

// Private Members
void SocketHandler::_initPorts()
{
	// inits this->_ports
	// std::map<std::string, ConfigStruct>::const_iterator confIt = this->_config->getCluster.begin();
	// std::map<std::string, ConfigStruct>::const_iterator confEnd = this->_config->getCluster.end();
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
			return;
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
		return;
	}
	struct kevent ev; // is this temp????
	EV_SET(&ev, _server_fd, EVFILT_READ, EV_ADD, 0, 0, NULL); // this needs to be looped for _serverFDs
	if (kevent(this->_kq, &ev, 1, NULL, 0, NULL) == -1)
	{
		std::cerr << RED << "Error adding server socket to kqueue" << std::endl;
		perror(NULL);
		std::cerr << RESET;
		return;
	}
}

void SocketHandler::_getEvents()
{
	int num_events = kevent(this->_kq, NULL, 0, this->_evList, MAX_EVENTS, NULL);
}

void SocketHandler::_acceptConnection()
{

}

void SocketHandler::_addClient()
{

}

void SocketHandler::_removeClient()
{

}

void SocketHandler::_getClient()
{

}

// Constructors
SocketHandler::SocketHandler(Config *config): _config(config)
{
	std::cout << "SocketHandler Default Constructor called" << std::endl;
}

SocketHandler::SocketHandler(const SocketHandler &src)
{
	std::cout << "SocketHandler Copy Constructor called" << std::endl;
	*this = src;
}

// Deconstructors
SocketHandler::~SocketHandler()
{
	/*CODE*/
	std::cout << "SocketHandler Deconstructor called" << std::endl;
}

// Overloaded Operators
SocketHandler &SocketHandler::operator=(const SocketHandler &src)
{
	std::cout << "SocketHandler Assignation operator called" << std::endl;
	if (this == &src)
		return *this;

	/*CODE*/
	return *this;
}

// Public Methods

// Getter

// Setter

