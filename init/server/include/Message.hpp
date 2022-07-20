#pragma once

#include <exception>
#include <string>
#include <utility> //std::pair
#include <map> //std::map
#include "Base.hpp"

#define PROTOCOL "HTTP/1.1"

class Message
{
	protected:
		std::string protocol;
		std::string body;
		bool hasBody;
		Message(const Message&);
		Message& operator=(const Message&);
		Message(void);
		void addHeaderField(const std::string&, const std::string&);
		std::map<std::string, std::string> headerFields;
	public:
		virtual ~Message(void) = 0;
		const std::string& getProtocol(void) const;
		const std::string& getBody(void) const;
		const std::map<std::string, std::string>& getHeaderFields(void) const;

	class HeaderFieldDuplicate : public std::exception
	{
		const char* what() const throw();
	};

	class InvalidStartLine : public std::exception
	{
		const char* what() const throw();
	};
};