#include "Request.hpp"

#include <string>
#include <vector>

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

void Request::createTokens(std::vector<std::string>& tokens, const std::string& message)
{
	std::string delimiter = " ";
	size_t last = 0;
	size_t next = 0;

	while ((next = message.find(delimiter, last)) != std::string::npos && tokens.size() < 3)
	{
		tokens.push_back(message.substr(last, next - last));
		last = next + 1;
	}
	tokens.push_back(message.substr(last));
	if (tokens.size() != 3)
		throw InvalidInput();
}

void Request::parseMessage(const std::string& message)
{
	std::vector<std::string> tokens;
	std::string method;
	std::string url;
	std::string protocol;
	
	if (message.empty())
		throw InvalidInput();
	createTokens(tokens, message);
	method = tokens[0];
	url = tokens[1];
	protocol = tokens[2];
	if (!isValidMethod(method))
		throw InvalidMethod();
	if (!isValidProtocol(protocol))
		throw InvalidProtocol();
	this->method = method;
	this->url = url;
	this->protocol = protocol;
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