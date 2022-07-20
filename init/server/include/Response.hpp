#pragma once

#include <string>
#include <map>
#include <iostream> //std::ostream
#include "Message.hpp"

#define DEFAULT_URI "default"

class Response : public Message
{
	private:
		int status;
		std::string statusMessage;
		std::map<int, std::string> messageMap;
		int fd;
		std::string url;
		void createMessageMap(void);
		bool isValidStatus(const int);
		int sendall(const int sock_fd, char *buffer, const int len) const;
		void createHeaderFields(void);
		void createBody(void);
		void createErrorBody(void);
	public:
		Response(std::string, int, int, std::string);
		~Response(void);

		// void setProtocol(const std::string&);
		// void setStatus(const int&);

		// const std::string& getProtocol(void) const;
		const int& getStatus(void) const;
		const std::string& getStatusMessage(void) const;

		std::string constructHeader(void);

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

