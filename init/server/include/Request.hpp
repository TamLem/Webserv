#pragma once

#include <string> //std::string
#include <iostream> //std::ostream
#include <set> //std::set
#include <vector> //std::vector
#include "Message.hpp"

#define INDEX_PATH "./pages/index.html"

class Request : public Message
{
	private:
		std::set<std::string> validMethods;
		std::string method;
		std::string url;
		// std::string protocol;
		void addMethods(void);
		void parseMessage(const std::string&);
		bool isValidMethod(std::string);
		void parseStartLine(std::istringstream&);
		void parseHeaderFields(std::istringstream&);
		void parseHeaderFieldLine(const std::string&);
		void parseBody(std::istringstream&);
		void createTokens(std::vector<std::string>&, const std::string&, const unsigned int&, const std::string&);
		void createHeaderTokens(std::vector<std::string>&, const std::string&, const unsigned int&, const std::string&);
		void setBodyFlag(void);
		// int _fd;
	public:
		Request(const std::string&);
		// Request(const std::string&, int fd);
		~Request(void);

		// void setMethod(const std::string&);
		// void setUrl(const std::string&);
		// void setProtocol(const std::string&);

		const std::string& getMethod(void) const;
		const std::string& getUrl(void) const;
		// const std::string& getProtocol(void) const;

	class InvalidNumberOfTokens : public std::exception
	{
		const char* what() const throw();
	};

	class EmptyMessage : public std::exception
	{
		const char* what() const throw();
	};

	class InvalidMethod : public std::exception
	{
		const char* what() const throw();
	};
};

std::ostream& operator<<(std::ostream&, const Request&);