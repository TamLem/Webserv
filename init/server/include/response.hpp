#ifndef __RESPONSE_HPP__
#define __RESPONSE_HPP__


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
class response
{
private:
	/* data */
public:
	response(){};
	~response(){};

	static void parse_request(std::string buffer, int fd);

};

size_t ft_intlen(int n)
{
	size_t i;

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

int sendall(int sock_fd, char *buffer, int len)
{
	int total;
	int bytesleft;
	int n;

	total = len;
	bytesleft = len;
	while (total > 0)
	{
		n = send(sock_fd, buffer, bytesleft, 0);
		if (n == -1)
		{
			perror("send");
			return (-1);
		}
		total -= n;
		bytesleft -= n;
		buffer += n;
	}
	return (0);
}

void cgi_response(std::string buffer, int fd)
{
	std::string file;

	std::cout << GREEN << "Executing CGI..." << std::endl;
	file = "test.php";
	int stdout_init = dup(STDOUT_FILENO);
	int pid = fork();
	if (pid == 0)
	{
		dup2(fd, STDOUT_FILENO);
		if(execlp("/usr/bin/php", "php", "cgi-bin/test.php", NULL) == -1)
		{
			std::cout << "error executing cgi" << std::endl;
		}
		exit(0);
	}
	dup2(stdout_init, STDOUT_FILENO);
	close(fd);
}

void response::parse_request(std::string buffer, int fd)
{
	if (strstr(buffer.c_str(), ".php"))
		cgi_response(buffer, fd);
	else if (strstr(buffer.c_str(), "GET") && strstr(buffer.c_str(), "favicon.ico"))
	{
		std::cout << GREEN << "@@@@@@ GET favicon.ico @@@@@" << RESET << std::endl
				  << std::endl;
		std::ifstream input("images/favicon.ico", std::ios::binary);
		if (input.is_open())
		{
			std::filebuf *pbuf = input.rdbuf();
			std::size_t size = pbuf->pubseekoff(0, input.end, input.in);
			// cout << size << endl;
			pbuf->pubseekpos(0, input.in);
			char *out_buffer = new char[size];
			pbuf->sgetn(out_buffer, size);
			input.close();
			size += strlen("HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: image/vdn.microsoft.icon\nContent-Length: \n\n") + ft_intlen(size);
			dprintf(fd, "HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: image/vdn.microsoft.icon\nContent-Length: %lu\n\n", size);
			// write(fd, out_buffer, size);
			sendall(fd, out_buffer, size);
			delete[] out_buffer;
		}
		else
			std::cerr << RED << "image favicon.ico not found" << RESET << std::endl;
		close(fd);
	}
	else if (strstr(buffer.c_str(), "GET") && strstr(buffer.c_str(), "/images/large.jpg"))
	{
		std::cout << GREEN << "@@@@@@ GET large.img @@@@@" << RESET << std::endl
				  << std::endl;
		std::ifstream input("/Users/tlemma/Documents/Webserv/main/init/server/images/large.jpg", std::ios::binary);
		std::filebuf *pbuf = input.rdbuf();
		std::size_t size = pbuf->pubseekoff(0, input.end, input.in);
		pbuf->pubseekpos(0, input.in);
		char *out_buffer = new char[size];
		pbuf->sgetn(out_buffer, size);
		input.close();
		size += strlen("HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: image/jpg\nContent-Length: \n\n") + ft_intlen(size);
		dprintf(fd, "HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: image/jpg\nContent-Length: %lu\n\n", size);
		// write(fd, out_buffer, size);
		sendall(fd, out_buffer, size);
		delete[] out_buffer;
		close(fd);
	}
	else if (strstr(buffer.c_str(), "GET"))
	{
		std::cout << GREEN << "@@@@@@ GET index @@@@@" << RESET << std::endl
				  << std::endl;
		std::ifstream myfile("/Users/tlemma/Documents/Webserv/main/init/server/pages/index.html");
		std::string myline;
		std::string out_buffer;
		std::size_t size = 0;
		if (myfile.is_open())
		{
			while (myfile.good())
			{ // equivalent to myfile.good()
				std::getline(myfile, out_buffer);
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

#endif