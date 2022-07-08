#include "Request.hpp"

#include <string>

void Request::parseMessage(const std::string& message)
{
	if (message.empty())
		throw InvalidInput();
	this->method = "GET";
	this->url = "/index.html";
	this->protocol = "HTTP/1.1";
}

Request::Request(const std::string& message)
{
	parseMessage(message);
}

Request::~Request(void)
{

}

std::ostream& operator<<(std::ostream& out, const Request& request)
{
	out << request.getMethod() << " "
	<< request.getUrl() << " "
	<< request.getProtocol();
	return (out);
}

void Request::setMethod(const std::string& method)
{
	this->method = method;
}

void Request::setUrl(const std::string& url)
{
	this->url = url;
}

void Request::setProtocol(const std::string& protocol)
{
	this->protocol = protocol;
}

const std::string& Request::getMethod(void) const
{
	return (this->method);
}

const std::string& Request::getUrl(void) const
{
	return (this->url);
}

const std::string& Request::getProtocol(void) const
{
	return (this->protocol);
}

// std::string Request::construct_header(void)
// {

// }

const char* Request::InvalidInput::what() const throw()
{
	return ("Exception: invalid input");
}