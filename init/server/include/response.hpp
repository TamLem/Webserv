#pragma once

#include <string>
#include <map>
#include <iostream> //std::ostream
#include "Message.hpp"

class Response : public Message
{
	private:
		// std::string protocol;
		int status;
		std::string statusMessage;
		std::map<int, std::string> messageMap;
		void createMessageMap(void);
		bool isValidStatus(int);
		size_t ft_intlen(int n);
		int sendall(int sock_fd, char *buffer, int len);
		int fd;
		std::string url;

	public:
		// Response(std::string, int);
		Response(std::string, int, int, std::string);
		~Response(void);

		// void setProtocol(const std::string&);
		// void setStatus(const int&);

		// const std::string& getProtocol(void) const;
		const int& getStatus(void) const;
		const std::string& getStatusMessage(void) const;

		std::string constructHeader(size_t size);

		void sendResponse(void);

	class InvalidStatus : public std::exception
	{
		const char* what() const throw();
	};

		class ERROR_404 : public std::exception
	{
		const char* what() const throw();
	};

};

std::ostream& operator<<(std::ostream&, const Response&);

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

