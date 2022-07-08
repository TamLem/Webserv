#include "Response.hpp"

#include <string> //std::string
#include <map> //std::map
#include <sstream> //std::stringstream


// Response::Response(const Response& other)
// {
	
// }

// Response& Response::operator=(const Response& other)
// {

// }

void Response::create_messages(void)
{
	this->messages[100] = "Continue";
	this->messages[200] = "OK";
	this->messages[404] = "Not Found";
}

bool Response::is_valid_protocol(std::string protocol)
{
	if (protocol == "HTTP/1.1")
		return (true);
	return (false);
}

bool Response::is_valid_status(int status)
{
	if (this->messages.count(status))
		return (true);
	return (false);
}

// Response::Response(void) //AE init
// {
// 	this->create_messages();
// }

Response::Response(std::string protocol, int status) //AE init
{
	this->create_messages();
	if (!is_valid_protocol(protocol))
		throw InvalidProtocol();
	if (!is_valid_status(status))
		throw InvalidStatus();
	this->protocol = protocol;
	this->status = status;
	this->message = this->messages[this->status];
}

Response::~Response(void)
{

}

std::ostream& operator<<(std::ostream& out, const Response& response)
{
	out << response.getProtocol() << " "
	<< response.getStatus() << " "
	<< response.getMessage();
	return (out);
}

// void Response::setProtocol(const std::string& protocol)
// {
// 	this->protocol = protocol;
// }

// void Response::setStatus(const int& status)
// {
// 	this->status = status;
// }

const std::string& Response::getProtocol(void) const
{
	return (this->protocol);
}

const int& Response::getStatus(void) const
{
	return (this->status);
}

const std::string& Response::getMessage(void) const
{
	return (this->message);
}

std::string Response::construct_header(void)
{
	std::stringstream stream;

	stream.clear();
	stream.str("");
	stream << this->protocol << " " << this->status << " " << this->message
	// << "\n" <<
	// "Date: " << responseClass.date << "\n" <<
	// "Server: " << responseClass.server << "\n" <<
	// "Content-Type: " << responseClass.type << "\n" <<
	// "Content-Length: " << responseClass.length
	<< "\n\n";

	return (stream.str());
}

const char* Response::InvalidProtocol::what() const throw()
{
	return ("Exception: invalid protocol");
}

const char* Response::InvalidStatus::what() const throw()
{
	return ("Exception: invalid status");
}