#pragma once

#include <exception>
#include <string>
#include <utility> //std::pair
#include <map> //std::map
#include "Base.hpp"

class Message
{
	protected:
		std::string protocol;
		std::string body;
		bool hasBody;
		//some container for header fields
		Message(const Message&);
		Message& operator=(const Message&);
		Message(void);
		bool isValidProtocol(std::string);
		void addHeaderField(const std::string&, const std::string&);
		std::map<std::string, std::string> headerFields;
	public:
		virtual ~Message(void) = 0;
		const std::string& getProtocol(void) const;
		const std::string& getBody(void) const;
		const std::map<std::string, std::string>& getHeaderFields(void) const;


	class InvalidProtocol : public std::exception
	{
		const char* what() const throw();
	};

	class HeaderFieldDuplicate : public std::exception
	{
		const char* what() const throw();
	};

	class InvalidStartLine : public std::exception
	{
		const char* what() const throw();
	};
};