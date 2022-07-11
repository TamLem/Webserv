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
	public:
		Response(std::string, int);
		~Response(void);

		// void setProtocol(const std::string&);
		// void setStatus(const int&);

		// const std::string& getProtocol(void) const;
		const int& getStatus(void) const;
		const std::string& getStatusMessage(void) const;

		std::string constructHeader(void);

	class InvalidStatus : public std::exception
	{
		const char* what() const throw();
	};
};

std::ostream& operator<<(std::ostream&, const Response&);