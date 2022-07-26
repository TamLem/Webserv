#include "Server.hpp"
#include "Config.hpp"
#include "Cgi/Cgi.hpp"

// Server::Server(void)
// {
// 	std::cout << "Server default constructor called for " << this << std::endl;
// 	handle_signals();
// }

Server::Server(Config* config) : _config(config)
{
	#ifdef SHOW_LOG
		std::cout << GREEN << "Server constructor called for " << this << RESET << std::endl;
	#endif
	handle_signals();
	_port = 8080;

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
	close(this->_server_fd);
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

void Server::stop(void){}

int Server::get_client(int fd)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i].fd == fd)
			return (i);
	}
	return (-1);
}

int Server::add_client(int fd, struct sockaddr_in addr)
{
	struct client c;
	c.fd = fd;
	c.addr = addr;
	_clients.push_back(c);
	return (_clients.size() - 1);
}

int Server::remove_client(int fd)
{
	int i = get_client(fd);
	if (i == -1)
		return (-1);
	_clients.erase(_clients.begin() + i);
	return (0);
}

void Server::run_event_loop(int kq)
{
// add those to private members
	struct kevent ev;
	// struct kevent evList[MAX_EVENTS];
	struct sockaddr_storage addr; // temp
//

	while(keep_running)
	{
// start _getEvents
		// int num_events = kevent(kq, NULL, 0, evList, MAX_EVENTS, NULL);
// end _getEvents
		for (int i = 0; i < num_events; i++)
		{
			if (evList[i].ident == _server_fd)
			{
// start _acceptConnections
				socklen_t addrlen = sizeof(addr);
				int fd = accept(_server_fd, (struct sockaddr *)&addr, &addrlen);
				if (fd < 0)
				{
					std::cerr << RED << "Error accepting connection" << std::endl;
					perror(NULL);
					std::cerr << RESET;
					continue;
				}
				#ifdef SHOW_LOG
					else
						std::cout << GREEN << "New connection on socket " << fd << RESET << std::endl;
				#endif
// start _addToEventLoop
				int set = 1;
				setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int)); // set socket to not SIGPIPE
				add_client(fd, *(struct sockaddr_in *)&addr);
				EV_SET(&ev, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
				kevent(kq, &ev, 1, NULL, 0, NULL);
// end _addToEventLoop
// end _acceptConnections
			}
			else if (evList[i].flags & EV_EOF) //handle client disconnect event
			{
// start _removeClient
				remove_client(evList[i].ident);
				EV_SET(&ev, evList[i].ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
				kevent(kq, &ev, 1, NULL, 0, NULL);
				#ifdef SHOW_LOG
					std::cout << GREEN << "Client " << evList[i].ident << " disconnected" << RESET << std::endl;
				#endif
// end _removeClient
			}
			else if (evList[i].flags & EVFILT_READ) //handle client read event
			{
// start _readFromClient
				int fd = evList[i].ident;
				int i = get_client(fd);
				if (i == -1)
				{
					std::cerr << RED << "Error getting client" << std::endl;
					perror(NULL);
					std::cerr << RESET;
					continue;
				}
			// run this in a loop to accept chuncks
				char buf[client_body_buffer_size]; // probably needs to be an ifstream to not overflow with enormous requests !!!!!!!!!!!
				int n = read(fd, buf, client_body_buffer_size);
			//
				if (n < 0)
				{
					std::cerr << RED << "Error reading from client" << std::endl;
					perror(NULL);
					std::cerr << RESET;
					continue;
				}
				buf[n] = '\0';
				#ifdef SHOW_LOG
					std::cout << YELLOW << "Received->" << RESET << buf << YELLOW << "<-Received" << RESET << std::endl;
				#endif

				handleRequest(buf, fd);
// end _readFromClient
			}
		}
	}
}

void Server::run()
{
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
	int kq = kqueue();
	if (kq == -1)
	{
		std::cerr << RED << "Error creating kqueue" << std::endl;
		perror(NULL);
		std::cerr << RESET;
		return;
	}
	struct kevent ev;
	EV_SET(&ev, _server_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	int ret = kevent(kq, &ev, 1, NULL, 0, NULL);
	if (ret == -1)
	{
		std::cerr << RED << "Error adding server socket to kqueue" << std::endl;
		perror(NULL);
		std::cerr << RESET;
		return;
	}
// end _initEventLoop
	run_event_loop(kq);
}

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

void Server::handleRequest(const std::string& buffer, int fd)
{
	try
	{
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

void cgi_handle(Request& request, std::string buf, int fd)
{
	Cgi newCgi(request);

	newCgi.printEnv();
	newCgi.cgi_response(buf, fd);
}