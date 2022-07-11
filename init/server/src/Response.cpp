#include "Response.hpp"

#include <string> //std::string
#include <map> //std::map
#include <sstream> //std::stringstream

void Response::createMessageMap(void)
{
	//1xx informational response
	this->messageMap[100] = "Continue";
	//2xx success
	this->messageMap[200] = "OK";
	//3xx redirection
	//4xx client errors
	this->messageMap[404] = "Not Found";
	//5xx server errors
}

bool Response::isValidStatus(int status)
{
	if (this->messageMap.count(status))
		return (true);
	return (false);
}

Response::Response(std::string protocol, int status)
{
	this->createMessageMap();
	if (!isValidProtocol(protocol))
		throw InvalidProtocol();
	if (!isValidStatus(status))
		throw InvalidStatus();
	this->protocol = protocol;
	this->status = status;
	this->statusMessage = this->messageMap[this->status];
	this->hasBody = true;
}

Response::~Response(void)
{

}

std::ostream& operator<<(std::ostream& out, const Response& response)
{
	out << response.getProtocol() << " "
	<< response.getStatus() << " "
	<< response.getStatusMessage();
	return (out);
}

const int& Response::getStatus(void) const
{
	return (this->status);
}

const std::string& Response::getStatusMessage(void) const
{
	return (this->statusMessage);
}

std::string Response::constructHeader(void)
{
	std::stringstream stream;

	stream.clear();
	stream.str("");
	stream << this->protocol << " " << this->status << " " << this->statusMessage
	// << "\n" <<
	// "Date: " << responseClass.date << "\n" <<
	// "Server: " << responseClass.server << "\n" <<
	// "Content-Type: " << responseClass.type << "\n" <<
	// "Content-Length: " << responseClass.length
	<< "\n\n";

	return (stream.str());
}

const char* Response::InvalidStatus::what() const throw()
{
	return ("Exception: invalid status");
}