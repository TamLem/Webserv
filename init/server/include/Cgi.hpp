#ifndef __CGI_HPP__
#define __CGI_HPP__

#include <iostream>
#include <string>
#include <unistd.h>
#include "Base.hpp"

void cgi_response(std::string buffer, int fd)
{
	std::string file;

	(void)buffer;
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

#endif