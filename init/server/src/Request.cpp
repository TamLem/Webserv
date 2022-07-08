#include "Request.hpp"

#include <string>

bool Request::isValidMethod(std::string method)
{
	if (this->validMethods.count(method))
		return (true);
	return (false);
}

void Request::addMethods(void)
{
	this->validMethods.insert("GET");
	this->validMethods.insert("POST");
	this->validMethods.insert("DELETE");
}

void Request::parseMessage(const std::string& message)
{
	std::string method;
	std::string url;
	std::string protocol;
	
	if (message.empty())
		throw InvalidInput();
	method = "GET"; //AE testing
	url = "/index.html"; //AE testing
	protocol = "HTTP/1.1"; //AE testing
	if (!isValidMethod(method))
		throw InvalidMethod();
	if (!isValidProtocol("HTTP/1.1"))
		throw InvalidProtocol();
	this->method = "GET";
	this->url = "/index.html";
	this->protocol = "HTTP/1.1";
}

Request::Request(const std::string& message)
{
	addMethods();
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

// const std::string& Request::getProtocol(void) const
// {
// 	return (this->protocol);
// }

const char* Request::InvalidInput::what() const throw()
{
	return ("Exception: invalid input");
}

const char* Request::InvalidMethod::what() const throw()
{
	return ("Exception: invalid method");
}