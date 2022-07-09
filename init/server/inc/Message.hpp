#pragma once

#include <exception>
#include <string>
#include <utility> //std::pair
#include <list> //std::list

#define CRLF "\n"

class Message
{
	protected:
		std::string protocol;
		std::string body;
		//some container for header fields
		Message(const Message&);
		Message& operator=(const Message&);
		Message(void);
		bool isValidProtocol(std::string);
	public:
		std::list<std::pair<std::string, std::string>> headerFields;
		virtual ~Message(void) = 0;
		const std::string& getProtocol(void) const;
		const std::string& getBody(void) const;

	class InvalidProtocol : public std::exception
	{
		const char* what() const throw();
	};
};