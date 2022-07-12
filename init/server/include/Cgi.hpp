#ifndef __CGI_HPP__
#define __CGI_HPP__

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include "Base.hpp"

void cgi_response(std::string buffer, int fd)
{
	std::string file;

	(void)buffer;
	std::cout << GREEN << "Executing CGI..." << std::endl;
	file = "cgi-bin/site/index.php";
	std::string header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
	send(fd, header.c_str(), header.size(), 0);
	int stdout_init = dup(STDOUT_FILENO);
	int pid = fork();
	if (pid == 0)
	{
		dup2(fd, STDOUT_FILENO);
		if(execlp("/usr/bin/php", "php", file.c_str(), NULL) == -1)
		{
			std::cout << "error executing cgi" << std::endl;
		}
		exit(0);
	}
	dup2(stdout_init, STDOUT_FILENO);
	close(fd);
}

#endif