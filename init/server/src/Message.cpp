#include "Message.hpp"

#include <iostream>

Message::Message(void)
{
	
}

Message::~Message(void)
{
	
}

const std::string& Message::getProtocol(void) const
{
	return (this->protocol);
}

const std::string& Message::getBody(void) const
{
	return (this->body);
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