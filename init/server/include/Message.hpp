#ifndef MESSAGE_HPP
#define MESSAGE_HPP
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
		bool isValidProtocol(const std::string&) const;
		std::map<std::string, std::string> headerFields;
	public:
		void addHeaderField(const std::string&, const std::string&);
		virtual ~Message(void) = 0;
		const std::string& getProtocol(void) const;
		const std::string& getBody(void) const;

		const std::map<std::string, std::string>& getHeaderFields(void) const;

	class BadRequest : public std::exception
	{
		virtual const char* what() const throw() = 0;
	};

	class HeaderFieldDuplicate : public Message::BadRequest
	{
		const char* what() const throw();
	};

	class InvalidStartLine : public Message::BadRequest
	{
		const char* what() const throw();
	};
};

#endif