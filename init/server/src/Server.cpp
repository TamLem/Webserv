#include "Server.hpp"
#include "Config.hpp"
#include "Cgi.hpp"

Server::Server(void)
{
	std::cout << "Server default constructor called for " << this << std::endl;
	handle_signals();
}

Server::Server(int port)
{
	std::cout << "Server constructor called for " << this << std::endl;
	handle_signals();
	_port = port;

	if ((_server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cerr << RED << "Error creating socket" << std::endl;
		perror(NULL);
		std::cerr << RESET;
		return;
	}

// Set socket reusable from Time-Wait state
	int val = 1;
	setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &val, 4); // how to implement | SO_NOSIGPIPE

// initialize server address struct
	struct sockaddr_in serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr)); // is memset allowed? !!!!!!!!!!!!!!!!!!!!
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

// bind socket to address
	if((bind(_server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
	{
		std::cerr << RED << "Error binding socket" << std::endl;
		perror(NULL);
		std::cerr << RESET;
		exit(EXIT_FAILURE);
	}
}

Server::~Server(void)
{
	close(this->_server_fd);
	std::cout << "server deconstructor called for " << this << std::endl;
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
	// 	std::cerr << RED << "SIGPIPE detected, will crash now" << RESET << std::endl;
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
	struct kevent ev;
	struct kevent evList[MAX_EVENTS];
	struct sockaddr_storage addr;

	while(keep_running)
	{
		int num_events = kevent(kq, NULL, 0, evList, MAX_EVENTS, NULL);
		for (int i = 0; i < num_events; i++)
		{
			if (evList[i].ident == _server_fd)
			{
				socklen_t addrlen = sizeof(addr);
				int fd = accept(_server_fd, (struct sockaddr *)&addr, &addrlen);
				if (fd < 0)
				{
					std::cerr << RED << "Error accepting connection" << std::endl;
					perror(NULL);
					std::cerr << RESET;
					continue;
				}
				else
					std::cout << GREEN << "New connection on socket " << fd << RESET << std::endl;
				int set = 1;
				setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int)); // set socket to not SIGPIPE
				add_client(fd, *(struct sockaddr_in *)&addr);
				EV_SET(&ev, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
				kevent(kq, &ev, 1, NULL, 0, NULL);
			}
			else if (evList[i].flags & EV_EOF) //handle client disconnect event
			{
				remove_client(evList[i].ident);
				EV_SET(&ev, evList[i].ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
				kevent(kq, &ev, 1, NULL, 0, NULL);
				std::cout << GREEN << "Client " << evList[i].ident << " disconnected" << RESET << std::endl;
			}
			else if (evList[i].flags & EVFILT_READ) //handle client read event
			{
				int fd = evList[i].ident;
				int i = get_client(fd);
				if (i == -1)
				{
					std::cerr << RED << "Error getting client" << std::endl;
					perror(NULL);
					std::cerr << RESET;
					continue;
				}
				char buf[1024]; // probably needs to be an ifstream to not overflow with enormous requests
				int n = read(fd, buf, 1024);
				if (n < 0)
				{
					std::cerr << RED << "Error reading from client" << std::endl;
					perror(NULL);
					std::cerr << RESET;
					continue;
				}
				buf[n] = '\0';
				std::cout << YELLOW << "Received->" << RESET << buf << YELLOW << "<-Received" << RESET << std::endl;

				if (std::string(buf).find(".php") != std::string::npos)
					cgi_response(buf, fd);
				else
					handle_static_request(buf, fd);
			}
		}
	}
}

void Server::run()
{
	if (listen(_server_fd, 5))
	{
		std::cerr << RED << "Error listening" << std::endl;
		perror(NULL);
		std::cerr << RESET;
		return;
	}
	else
		std::cout << "Listening on port " << _port << std::endl;
// create a kqueue
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
	run_event_loop(kq);
}

void Server::handleGET(int status, int fd, const std::string& uri)
{
	Response.init(status, fd, uri);
	Response.createBody();
	Response.createHeaderFields();
	Response.sendResponse();
}

void Server::handlePOST(int status, int fd, const Request& newRequest)
{
	std::ofstream outFile;
	outFile.open("./uploads/" + newRequest.getBody());
	if (outFile.is_open() == false)
		throw std::exception();
	outFile << newRequest.getBody() << "'s content.";
	outFile.close();
	Response.init(status, fd, "./pages/post_test.html");
	Response.createBody();
	Response.createHeaderFields();
	Response.sendResponse();
}

void Server::handleERROR(int status, int fd)
{
	Response.init(status, fd, ""); //AE make overload instead of passing ""
	Response.createErrorBody();
	Response.createHeaderFields();
	Response.sendResponse();
}

void Server::handle_static_request(const std::string& buffer, int fd)
{
	try
	{
		Request newRequest(buffer);
		if (newRequest.getMethod() == "POST")
			handlePOST(200, fd, newRequest);
		else
			handleGET(200, fd, newRequest.getUri());
	}
	catch (Request::InvalidMethod& e)
	{
		handleERROR(501, fd);
	}
	catch (Request::InvalidProtocol& e)
	{
		handleERROR(505, fd);
	}
	catch (Response::ERROR_404& e)
	{
		handleERROR(404, fd);
	}
	catch (Message::BadRequest& e)
	{
		handleERROR(400, fd);
	}
	catch (std::exception& e)
	{
		handleERROR(500, fd);
	}
}
