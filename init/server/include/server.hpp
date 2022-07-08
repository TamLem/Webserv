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
			struct sockaddr_in serv_addr;
			memset(&serv_addr, '0', sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port = htons(port);
			serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

			if((bind(_server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
			{
				std::cerr << RED << "Error binding socket" << std::endl;
				perror(NULL);
				std::cerr << RESET;
				exit(EXIT_FAILURE);
			}
		}
		~Server()
		{}
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

		size_t	ft_intlen(int n)
		{
			size_t	i;

			i = 0;
			if (n == -2147483648)
				return (10);
			if (n >= 0 && n <= 9)
				return (1);
			if (n < 0)
			{
				n = n * -1;
				i++;
			}
			while (n > 0)
			{
				n = n / 10;
				i++;
			}
			return (i);
		}

		void parse_request(std::string buffer, int fd)
		{
			if (strstr(buffer.c_str(), "GET") && strstr(buffer.c_str(), "favicon.ico"))
			{
				std::cout << GREEN << "@@@@@@ GET favicon.ico @@@@@" << RESET << std::endl << std::endl;
				std::ifstream input("images/favicon.ico", std::ios::binary);
				if (input.is_open())
				{
					std::filebuf *pbuf = input.rdbuf();
					std::size_t size = pbuf->pubseekoff(0,input.end,input.in);
					// cout << size << endl;
					pbuf->pubseekpos(0,input.in);
					char *out_buffer = new char[size];
					pbuf->sgetn(out_buffer,size);
					input.close();
					size += strlen("HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: image/vdn.microsoft.icon\nContent-Length: \n\n") + ft_intlen(size);
					dprintf(fd, "HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: image/vdn.microsoft.icon\nContent-Length: %lu\n\n", size);
					write(fd, out_buffer, size);
					delete[] out_buffer;
				}
				else
					std::cerr << RED << "image favicon.ico not found" << RESET << std::endl;
				close(fd);
			}
			else if (strstr(buffer.c_str(), "GET") && strstr(buffer.c_str(), "/images/large.jpg"))
			{
				std::cout << GREEN << "@@@@@@ GET large.img @@@@@" << RESET << std::endl << std::endl;
				std::ifstream input("/Users/tblaase/Documents/webserv/init/server/images/large.jpg", std::ios::binary);
				std::filebuf *pbuf = input.rdbuf();
				std::size_t size = pbuf->pubseekoff(0,input.end,input.in);
				pbuf->pubseekpos(0,input.in);
				char *out_buffer = new char[size];
				pbuf->sgetn(out_buffer,size);
				input.close();
				size += strlen("HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: image/jpg\nContent-Length: \n\n") + ft_intlen(size);
				dprintf(fd, "HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: image/jpg\nContent-Length: %lu\n\n", size);
				write(fd, out_buffer, size);
				delete[] out_buffer;
				close(fd);
			}
			else if (strstr(buffer.c_str(), "GET"))
			{
				std::cout << GREEN << "@@@@@@ GET index @@@@@" << RESET << std::endl << std::endl;
				std::ifstream myfile("/Users/tblaase/Documents/webserv/init/server/pages/index.html");
				std::string myline;
				std::string out_buffer;
				std::size_t size = 0;
				if (myfile.is_open())
				{
					while (myfile.good())
					{ // equivalent to myfile.good()
						std::getline (myfile, out_buffer);
						myline.append(out_buffer + "\n");
					}
					size = myline.length();
					size += strlen("HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: text/html\nContent-Length: \n\n") + ft_intlen(size);
					dprintf(fd, "HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: text/html\nContent-Length: %lu\n\n", size);
					write(fd, myline.c_str(), myline.length());
					myfile.close();
				}
				else
				{
					std::cerr << RED << "Couldn't open file" << std::endl;
					perror(NULL);
					std::cerr << RESET;
					size_t size = strlen("HTTP/1.1 404 \nServer: localhost:8080\nContent-Type: image/jpg\nContent-Length: \n\n");
					size += ft_intlen(size);
					dprintf(fd, "HTTP/1.1 404 \nServer: localhost:8080\nContent-Type: image/jpg\nContent-Length: %lu\n\n", size);
				}
				close(fd);
			}
			else
			{
				std::cout << "not the correct key" << std::endl;
			}
		}

#ifdef USE_KQUEUE
		void run_event_loop(int kq)
		{
			struct kevent ev;
			struct kevent evList[MAX_EVENTS];
			struct sockaddr_storage addr;

			while(true)
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
						parse_request(buf, fd);
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