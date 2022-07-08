#include "Message.hpp"

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

const char* Message::InvalidProtocol::what() const throw()
{
	return ("Exception: invalid protocol");
}

bool Message::isValidProtocol(std::string protocol)
{
	if (protocol == "HTTP/1.1")
		return (true);
	return (false);
}