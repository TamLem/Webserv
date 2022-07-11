#include "Request.hpp"

#include <string>
#include <vector>
#include <sstream> //std::istringstream
#include <ios> //ios::eof

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

void Request::createTokens(std::vector<std::string>& tokens, const std::string& message, const unsigned int& count, const std::string& delimiter)
{
	// std::string delimiter = " ";
	size_t last = 0;
	size_t next = 0;

	while ((next = message.find(delimiter, last)) != std::string::npos && tokens.size() < count + 1)
	{
		tokens.push_back(message.substr(last, next - last));
		last = next + delimiter.length();
	}
	next = message.find("\r", last); // AE make sure this exists
	tokens.push_back(message.substr(last, next - last));
	if (tokens.size() != count)
		throw InvalidNumberOfTokens();
}

void Request::createHeaderTokens(std::vector<std::string>& tokens, const std::string& message, const unsigned int& count, const std::string& delimiter)
{
	// std::string delimiter = " ";
	size_t last = 0;
	size_t next = 0;

	while ((next = message.find(delimiter, last)) != std::string::npos && tokens.size() < count + 1)
	{
		tokens.push_back(message.substr(last, next - last));
		last = next + delimiter.length();
	}
	tokens.push_back(message.substr(last));
	if (tokens.size() != count)
		throw InvalidNumberOfTokens();
}

void Request::parseStartLine(std::istringstream& stream)
{
	std::vector<std::string> tokens;
	std::string line;

	std::getline(stream, line);
	createTokens(tokens, line, 3u, " ");
	// std::cout << "MY REQUEST PROTOCOL:" << tokens[2] << "." << std::endl;
	if (!isValidMethod(tokens[0]))
		throw InvalidMethod();
	if (!isValidProtocol(tokens[2]))
		throw InvalidProtocol();
	this->method = tokens[0];
	this->url = tokens[1];
	this->protocol = tokens[2];
}

void Request::parseHeaderFieldLine(const std::string& line)
{
	std::vector<std::string> tokens;

	createHeaderTokens(tokens, line, 2u, ": ");
	if (this->headerFields.count(tokens[0]))
		throw HeaderFieldDuplicate();
	this->headerFields[tokens[0]] = tokens[1];
}

void Request::parseHeaderFields(std::istringstream& stream)
{
	while (stream.eof() == false)
	{
		std::string line;
		
		std::getline(stream, line);
		if (line == "\r")
		{
			break ;
		}
		parseHeaderFieldLine(line);
	}
}

void Request::parseBody(std::istringstream& stream)
{
	int length = 0;
	
	while (stream.eof() == false)
	// while (length < this->headerFields["Content-Length"]) AE length check, how to ptotect agains invalid chars?
	{
		std::string line;
		
		std::getline(stream, line);
		length += line.length();
		this->body += line;
	}
}

void Request::setBodyFlag(void)
{
	if (this->headerFields.count("Content-Length") || this->headerFields.count("Transfer-Encoding")) //AE handle caseinsensitivity
		this->hasBody = true;
}

void Request::parseMessage(const std::string& message)
{
	if (message.empty())
		throw EmptyMessage();
	std::istringstream stream (message);
	parseStartLine(stream);
	parseHeaderFields(stream);
	setBodyFlag();
	if (this->hasBody == true)
		parseBody(stream);
}

Request::Request(const std::string& message)
{
	this->hasBody = false;
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
	<< request.getProtocol() << "\n";
	for (std::map<std::string, std::string>::const_iterator it=request.headerFields.begin(); it != request.headerFields.end(); ++it)
	{
		out << it->first << ": "
		<< it->second << "\n";
	}
	out << request.getBody() << std::endl;
	return (out);
}

// void Request::setMethod(const std::string& method)
// {
// 	this->method = method;
// }

// void Request::setUrl(const std::string& url)
// {
// 	this->url = url;
// }

// void Request::setProtocol(const std::string& protocol)
// {
// 	this->protocol = protocol;
// }

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

const char* Request::InvalidNumberOfTokens::what() const throw()
{
	return ("Exception: invalid number of tokens");
}

const char* Request::EmptyMessage::what() const throw()
{
	return ("Exception: empty message");
}

const char* Request::InvalidMethod::what() const throw()
{
	return ("Exception: invalid method");
}