#pragma once

#include <string>
#include <map>
#include <iostream> //std::ostream
#include "Message.hpp"

class Response : public Message
{
	private:
		std::string status;
		std::string statusMessage;
		std::map<std::string, std::string> messageMap;
		int fd;
		std::string uri;
		void createMessageMap(void);
		bool isValidStatus(const std::string&);
		int sendall(const int sock_fd, char *buffer, const int len) const;
	public:
		Response(void);
		// Response(int, int, const std::string&);
		// Response(int, int);
		~Response(void);

		// void setProtocol(const std::string&);
		// void setStatus(const int&);

		// const std::string& getProtocol(void) const;
		const std::string& getStatus(void) const;
		const std::string& getStatusMessage(void) const;
		const std::map<std::string, std::string>& getMessageMap(void) const;

		std::string constructHeader(void);

		void clear(void);
		void init(const std::string&, int, const std::string&);
		void createHeaderFields(void);
		void createBody(void);
		void createErrorBody(void);
		void sendResponse(void);

	class InvalidStatus : public std::exception
	{
		const char* what() const throw();
	};

	class ERROR_404 : public Message::BadRequest
	{
		const char* what() const throw();
	};

};

std::ostream& operator<<(std::ostream&, const Response&);

