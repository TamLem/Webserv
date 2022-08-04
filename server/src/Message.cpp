#include "Message.hpp"

#include <iostream>

Message::Message(void)
{
	#ifdef SHOW_CONSTRUCTION
		std::cout << GREEN << "Message Default Constructor called for " << this << RESET << std::endl;
	#endif
}

Message::~Message(void)
{
	#ifdef SHOW_CONSTRUCTION
		// std::cout << "headerFields: " << this->headerFields.size() << std::endl;
		std::cout << RED << "Message Deconstructor called for " << this << RESET << std::endl;
	#endif
}


bool Message::isValidProtocol(const std::string& protocol) const
{
	if (protocol == PROTOCOL)
		return (true);
	return (false);
}

void Message::addHeaderField(const std::string& key, const std::string& value)
{
	if (this->headerFields.count(key))
		throw HeaderFieldDuplicate();
	this->headerFields[key] = value;
}

// void Message::setTarget(const std::string& target)
// {
// 	this->target = target;
// }

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

const char* Message::HeaderFieldDuplicate::what() const throw()
{
	return ("400");
}

const char* Message::InvalidStartLine::what() const throw()
{
	return ("400");
}