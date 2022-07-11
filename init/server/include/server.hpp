#ifndef SERVER_HPP
#define SERVER_HPP

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
#include "response.hpp"

#if defined(__FreeBSD__) || defined(__APPLE__) || defined(__OpenBSD__) || defined(__NetBSD__)
	#define USE_KQUEUE
	#include <sys/event.h>
#elif defined(__linux__)
	#define USE_POLL
	#include <sys/poll.h>
#endif

#define RESET "\033[0m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RED "\033[31m"

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
		Server(int port)
		{
			struct sockaddr_in cli_addr;
			_port = port;

			if ((_server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				std::cerr << RED << "Error creating socket" << std::endl;
				perror(NULL);
				std::cerr << RESET;
				return;
			}

			/* Set socket reusable from Time-Wait state */
			int val = 1;
			setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &val, 4);

			/* initialize server address struct */
			struct sockaddr_in serv_addr;
			memset(&serv_addr, '0', sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port = htons(port);
			serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

			/* bind socket to address */
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
		}
		void stop();
		int get_client(int fd)
		{
			for (int i = 0; i < _clients.size(); i++)
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

#ifdef USE_KQUEUE
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
						char buf[1024];
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
						response::parse_request(buf, fd);
					}
				}
			}
		}
#elif defined(USE_POLL)
		// insert poll implementation here
#endif

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
#ifdef USE_KQUEUE
			/*create a kqueue*/
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
#elif defined(USE_POLL)
			// insert poll implementation here
#endif
		}
	private:
		int _port;
		int _server_fd;
		std::vector <client> _clients;
};

#endif