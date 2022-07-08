#pragma once

#include <exception>
#include <string>

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
		virtual ~Message(void) = 0;
		const std::string& getProtocol(void) const;

	class InvalidProtocol : public std::exception
	{
		const char* what() const throw();
	};
};