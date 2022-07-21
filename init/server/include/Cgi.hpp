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
	int pipefd[2];

	(void)buffer;
	std::cout << GREEN << "Executing CGI..." << std::endl;
	file = "index.php";
	int stdout_init = dup(STDOUT_FILENO);
	pipe(pipefd);
	dup2(pipefd[1], STDOUT_FILENO);
	close(pipefd[1]);
	int pid = fork();
	if (pid == 0)
	{
		chdir("cgi-bin/site");
		if(execlp("/usr/bin/php", "php", file.c_str(), NULL) == -1)
		{
			std::cout << "error executing cgi" << std::endl;
		}
		exit(0);
	}
	wait(NULL);
	dup2(stdout_init, STDOUT_FILENO);
	std::string header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
	send(fd, header.c_str(), header.size(), 0);
	char buf[1024];
	int n;
	while((n = read(pipefd[0], buf, 1024)))
	{
		send(fd, buf, n, 0);
	}
	close(pipefd[0]);
	close(fd);
}

#endif