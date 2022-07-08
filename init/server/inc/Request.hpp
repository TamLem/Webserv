#pragma once

#include <string> //std::string
#include <iostream> //std::ostream
#include <set> //std::set
#include "Message.hpp"

class Request : public Message
{
	private:
		std::set<std::string> validMethods;
		std::string method;
		std::string url;
		// std::string protocol;
		void addMethods(void);
		void parseMessage(const std::string& message);
		bool isValidMethod(std::string);
	public:
		Request(const std::string&);
		~Request(void);

		void setMethod(const std::string&);
		void setUrl(const std::string&);
		void setProtocol(const std::string&);

		const std::string& getMethod(void) const;
		const std::string& getUrl(void) const;
		// const std::string& getProtocol(void) const;

	class InvalidInput : public std::exception
	{
		const char* what() const throw();
	};

	class InvalidMethod : public std::exception
	{
		const char* what() const throw();
	};
};

std::ostream& operator<<(std::ostream&, const Request&);