#include "Server.hpp"

// Exceptions
const char* Server::InternalServerErrorException::what(void) const throw()
{
	return ("500");
}

const char* Server::BadRequestException::what(void) const throw()
{
	return ("400");
}

const char* Server::FirstLineTooLongException::what(void) const throw()
{
	return ("414");
}

const char* Server::InvalidHex::what() const throw()
{
	return ("400");
}

const char* Server::MethodNotAllowed::what() const throw()
{
	return ("405");
}

const char* Server::LengthRequiredException::what() const throw()
{
	return ("411");
}

const char* Server::ContentTooLargeException::what() const throw()
{
	return ("413");
}

const char* Server::ClientDisconnect::what() const throw()
{
	return ("client disconnect");
}
