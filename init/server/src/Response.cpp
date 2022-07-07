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

Response::Response(void) //AE init
{
	this->create_messages();
}

Response::~Response(void)
{

}

// std::ostream& operator<<(std::ostream& stream, const Response& response)
// {
// 	stream << response.get
// 	return (stream);
// }

void Response::setProtocol(const std::string& protocol)
{
	this->protocol = protocol;
}

void Response::setStatus(const int& status)
{
	this->status = status;
}

const std::string& Response::getProtocol(void) const
{
	return (this->protocol);
}

const int& Response::getStatus(void) const
{
	return (this->status);
}

std::string Response::construct_header(void)
{
	std::stringstream stream;

	stream.clear();
	stream.str("");
	stream << this->protocol << " " << this->status << " " << this->messages[this->status]
	// << "\n" <<
	// "Date: " << responseClass.date << "\n" <<
	// "Server: " << responseClass.server << "\n" <<
	// "Content-Type: " << responseClass.type << "\n" <<
	// "Content-Length: " << responseClass.length
	<< "\n\n";

	return (stream.str());
}