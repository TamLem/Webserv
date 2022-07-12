#include "Message.hpp"

#include <iostream>

Message::Message(void)
{
	
}

Message::~Message(void)
{
	
}

void Message::addHeaderField(const std::string& key, const std::string& value)
{
	if (this->headerFields.count(key))
		throw HeaderFieldDuplicate();
	this->headerFields[key] = value;
}

const std::string& Message::getProtocol(void) const
{
	return (this->protocol);
}

const std::string& Message::getBody(void) const
{
	return (this->body);
}

const std::map<std::string, std::string>& Message::getHeaderFields(void) const
{
	return (this->headerFields);
}

const char* Message::InvalidProtocol::what() const throw()
{
	return ("Exception: invalid protocol");
}

const char* Message::HeaderFieldDuplicate::what() const throw()
{
	return ("Exception: header field duplicate");
}

bool Message::isValidProtocol(std::string protocol)
{
	if (protocol == "HTTP/1.1")
		return (true);
	return (false);
}