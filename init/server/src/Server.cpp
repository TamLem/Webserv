#include "Server.hpp"
#include "Config.hpp"
#include "Cgi/Cgi.hpp"

// Server::Server(void)
// {
// 	std::cout << "Server default constructor called for " << this << std::endl;
// 	handle_signals();
// }

Server::Server(Config* config): _config(config), _socketHandler(new SocketHandler(_config))
{
	#ifdef SHOW_LOG
		std::cout << GREEN << "Server constructor called for " << this << RESET << std::endl;
	#endif
	handle_signals();
	// _port = 8080;

// start _initMainPorts
	// if ((_server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	// {
	// 	std::cerr << RED << "Error creating socket" << std::endl;
	// 	perror(NULL);
	// 	std::cerr << RESET;
	// 	return;
	// }


	// // Set socket reusable from Time-Wait state
	// int val = 1;
	// setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &val, 4); // is SO_NOSIGPIPE needed here ???????

	// // initialize server address struct
	// struct sockaddr_in serv_addr;
	// memset(&serv_addr, '0', sizeof(serv_addr)); // is memset allowed? !!!!!!!!!!!!!!!!!!!!
	// serv_addr.sin_family = AF_INET;
	// serv_addr.sin_port = htons(_port);
	// serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// // bind socket to address
	// if((bind(_server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
	// {
	// 	std::cerr << RED << "Error binding socket" << std::endl;
	// 	perror(NULL);
	// 	std::cerr << RESET;
	// 	exit(EXIT_FAILURE); // maybe change to return or throw
	// }
// end _initMainPorts
}

Server::~Server(void)
{
	delete _socketHandler;
	#ifdef SHOW_LOG
		std::cout << RED << "server deconstructor called for " << this << RESET << std::endl;
	#endif
}


void Server::handle_signal(int sig)
{
	if (sig == SIGINT)
	{
		std::cerr << BLUE << "SIGINT detected, terminating server now" << RESET << std::endl;
		keep_running = 0;
	}
	// else if (sig == SIGPIPE)
	// {
	// 	std::cerr << RED << "SIGPIPE detected, will end now" << RESET << std::endl;
	// 	keep_running = 0;
	// }
}

// check if signal is forbidden!!!!!!!!!!!!!!!!!
void	Server::handle_signals(void)
{
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	// signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, handle_signal);
	// signal(SIGPIPE, handle_signal);
}

// int Server::get_client(int fd)
// {
// // 	for (size_t i = 0; i < _clients.size(); i++)
// // 	{
// // 		if (_clients[i].fd == fd)
// // 			return (i);
// // 	}
// // 	return (-1);
// }

// int Server::add_client(int fd, struct sockaddr_in addr)
// {
// 	// struct client c;
// 	// c.fd = fd;
// 	// c.addr = addr;
// 	// _clients.push_back(c);
// 	// return (_clients.size() - 1);
// }

// int Server::remove_client(int fd)
// {
// 	// int i = get_client(fd);
// 	// if (i == -1)
// 	// 	return (-1);
// 	// _clients.erase(_clients.begin() + i);
// 	// return (0);
// }

void Server::runEventLoop()
{
// add those to private members
	// struct kevent ev;
	// struct kevent evList[MAX_EVENTS];
	// struct sockaddr_storage addr; // temp

	while(keep_running)
	{
		this->_socketHandler->getEvents();
		for (int i = 0; i < this->_socketHandler->getNumEvents() ; ++i)
		{
			std::cout << "no. events: " << this->_socketHandler->getNumEvents() << " ev:" << i << std::endl;
			this->_socketHandler->acceptConnection(i);
			this->_socketHandler->removeClient(i);
			if (this->_socketHandler->readFromClient(i) == true)
			{
				this->_readRequestHead(this->_socketHandler->getFD()); // read 1024 charackters or if less until /r/n/r/n is found
				handleRequest(this->_requestHead, this->_socketHandler->getFD());
				continue;
			}
		}
	}
}

// void Server::run()
// {
	// this->_socketHandler = SocketHandler(this->_config);
// start _listenMainSockets
	// if (listen(_server_fd, 5))
	// {
	// 	std::cerr << RED << "Error listening" << std::endl;
	// 	perror(NULL);
	// 	std::cerr << RESET;
	// 	return;
	// }
	// else
	// 	std::cout << "Listening on port " << _port << std::endl;
// end _listenMainSockets

// start _initEventLoop
	// int kq = kqueue();
	// if (kq == -1)
	// {
	// 	std::cerr << RED << "Error creating kqueue" << std::endl;
	// 	perror(NULL);
	// 	std::cerr << RESET;
	// 	return;
	// }
	// struct kevent ev;
	// EV_SET(&ev, _server_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	// int ret = kevent(kq, &ev, 1, NULL, 0, NULL);
	// if (ret == -1)
	// {
	// 	std::cerr << RED << "Error adding server socket to kqueue" << std::endl;
	// 	perror(NULL);
	// 	std::cerr << RESET;
	// 	return;
	// }
// end _initEventLoop
	// run_event_loop();
// }

void Server::handleGET(const std::string& status, int fd, const std::string& uri)
{
	_response.init(status, fd, uri);
	_response.createBody();
	_response.createHeaderFields();
	_response.sendResponse();
}

void Server::handlePOST(const std::string& status, int fd, const Request& newRequest)
{
	std::ofstream outFile;
	outFile.open("./uploads/" + newRequest.getBody());
	if (outFile.is_open() == false)
		throw std::exception();
	outFile << newRequest.getBody() << "'s content. Server: " << this->_config->getConfigStruct("weebserv").serverName;
	outFile.close();
	_response.init(status, fd, "./pages/post_test.html");
	_response.createBody();
	_response.createHeaderFields();
	_response.sendResponse();
}

void Server::handleERROR(const std::string& status, int fd)
{
	_response.init(status, fd, ""); //AE make overload instead of passing ""
	_response.createErrorBody();
	_response.createHeaderFields();
	_response.sendResponse();
}

void Server::handleRequest(const std::string& buffer, int fd) // maybe breaks here
{
	try
	{
		// std::cout << "Buffer contains >" << buffer << "<" << std::endl;
		Request newRequest(buffer);
		if (buffer.find("/cgi/") != std::string::npos)
			cgi_handle(newRequest, buffer, fd);
		else if (newRequest.getMethod() == "POST")
			handlePOST("200", fd, newRequest);
		else
			handleGET("200", fd, newRequest.getUri());
	}
	catch (std::exception& exception)
	{
		std::string code = exception.what();
		if (_response.getMessageMap().count(code) != 1)
			code = "500";
		handleERROR(code, fd);
	}
}

bool Server::_crlftwoFound()
{
	if (this->_requestHead.find(CRLFTWO) != std::string::npos)
		return (true);
	else
		return (false);
}

bool Server::_isPrintableAscii(char c)
{
	if (c > 126 || c < 0)
		return (false);
	else
		return (true);
}

void Server::_readRequestHead(int fd)
{
	// read 1024 charackters or if less until /r/n/r/n is found
	this->_requestHead.clear();
	size_t charsRead = 0;
	bool firstLineBreak = false;
	int n = 0;
	char buffer[2];
	while (charsRead < MAX_REQUEST_HEADER_SIZE)
	{
		n = read(fd, buffer, 1);
		if (n < 0)
		{
			std::cerr << RED << "READING FROM FD " << fd << " FAILED" << std::endl;
			perror(NULL);
			std::cerr << RESET << std::endl;
			// throw InternatServerErrorException(); ?? send some error page to client
			exit(EXIT_FAILURE);
		}
		else if (n == 0)
			break ;
		else // append one character from client request a
		{
			buffer[1] = '\0';
			if (!this->_isPrintableAscii(buffer[0]))
			{
				std::cout << RED << "NON-ASCII CHAR FOUND IN REQUEST" << RESET << std::endl;
				// throw NonAsciiFoundException(); ?? send some error page to client
				exit(EXIT_FAILURE);
			}
			this->_requestHead.append(buffer);
			++charsRead;
			if (this->_crlftwoFound() == true)
				break ;
		}
		if (firstLineBreak == false && this->_requestHead.find(CRLF) != std::string::npos)
		{
			firstLineBreak = true;
		}
		if (firstLineBreak == false && charsRead >= MAX_REQUEST_LINE_SIZE)
		{
			std::cout << RED << "FIRST LINE TOO LONG" << RESET << std::endl;
			// throw FirstLineTooLongException(); ?? send some error page to client
			exit(EXIT_FAILURE);
		}
	}
	if (charsRead <= MAX_REQUEST_HEADER_SIZE && this->_crlftwoFound() == true)
	{
		#ifdef SHOW_LOG
			std::cout << YELLOW << "Received->" << RESET << this->_requestHead << YELLOW << "<-Received" << RESET << std::endl;
		#endif
	}
	else /*if (charsRead >= MAX_REQUEST_HEADER_SIZE && this->_crlftwoFound() == false)*/
	{
		std::cout << RED << "HEAD BIGGER THAN " << MAX_REQUEST_HEADER_SIZE << " OR NO CRLFTWO FOUND (incomplete request)" << RESET << std::endl;
		// throw HeadTooBigException(); ?? send some error page to client
		exit(EXIT_FAILURE);
	}
}

void cgi_handle(Request& request, std::string buf, int fd)
{
	Cgi newCgi(request);

	newCgi.printEnv();
	newCgi.cgi_response(buf, fd);
}