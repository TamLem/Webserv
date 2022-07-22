#ifndef SERVER_HPP
#define SERVER_HPP

/* system includes */
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <sys/event.h>
#include <map>
#include <csignal> // check if forbidden !!!!!!!!!!

/* our includes */
#include "Response.hpp"
#include "Request.hpp"
#include "Cgi.hpp"
#include "SingleServerConfig.hpp"

#include "Base.hpp"

// Forbidden includes
#include <errno.h>

struct client
{
	int fd;
	struct sockaddr_in addr;
};

#define MAX_EVENTS 128

static volatile int keep_running = 1;

class Server
{
	public:
		Server()
		{
			std::cout << "Server default constructor called for " << this << std::endl;
		}

		Server(int port)
		{
			std::cout << "Server constructor called for " << this << std::endl;
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
		~Server()
		{
			close(this->_server_fd);
			std::cout << "server deconstructor called for " << this << std::endl;
		}

		void stop();

		int get_client(int fd)
		{
			for (size_t i = 0; i < _clients.size(); i++)
			{
				if (_clients[i].fd == fd)
					return (i);
			}
			return (-1);
		}

		int add_client(int fd, struct sockaddr_in addr)
		{
			struct client c;
			c.fd = fd;
			c.addr = addr;
			_clients.push_back(c);
			return (_clients.size() - 1);
		}

		int remove_client(int fd)
		{
			int i = get_client(fd);
			if (i == -1)
				return (-1);
			_clients.erase(_clients.begin() + i);
			return (0);
		}

		void run_event_loop(int kq)
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

		void run(){
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

		void handle_static_request(const std::string&, int);
		void handleGET(int, int, const std::string&);
		void handlePOST(int, int, const Request&);
		void handleERROR(int, int);
	private:
		size_t _port;
		size_t _server_fd;
		std::vector <client> _clients;
};

#endif