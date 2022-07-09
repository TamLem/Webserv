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

void Request::createTokens(std::vector<std::string>& tokens, const std::string& message, const int& count, const std::string& delimiter)
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
		throw InvalidInput();
}

void Request::parseStartLine(const std::string& startLine)
{
	std::vector<std::string> tokens;

	createTokens(tokens, startLine, 3, " ");
	if (!isValidMethod(tokens[0]))
		throw InvalidMethod();
	if (!isValidProtocol(tokens[2]))
		throw InvalidProtocol();
	this->method = tokens[0];
	this->url = tokens[1];
	this->protocol = tokens[2];
}

void Request::parseHeaderFields(const std::string& line)
{
	std::vector<std::string> tokens;

	createTokens(tokens, line, 2, ": ");
	// std::pair<std::string, std::string> pair;
	// pair = std::make_pair("tokens[0]", "tokens[1]");
	// std::pair<std::string&, std::string&> = std::make_pair<tokens[0], tokens[1]>;
	this->headerFields.push_back(std::make_pair(tokens[0], tokens[1]));
	// this->headerFields.push_back(std::make_pair<tokens[0], tokens[1]>);
}

void Request::parseMessage(const std::string& message)
{
	std::string line;


	if (message.empty())
		throw InvalidInput();
	std::istringstream stream (message);
	std::getline(stream, line);
	parseStartLine(line);
	while (stream.eof() == false)
	{
		std::getline(stream, line);
		// std::cout << "LINE:" << line << "." << std::endl;
		if (line.empty())
			break ;
		parseHeaderFields(line);
	}
	while (stream.eof() == false)
	{
		std::getline(stream, line);
		this->body += line;
	}
	// while (stream.eof() == false)
	// {
	// 	std::getline(stream, line);
	// 	parseHeaderFields(line);
	// }
	// parseStartLine(stream.getline(line,  CRLF));
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
	<< request.getProtocol() << "\n";
	for (std::list<std::pair<std::string, std::string>>::const_iterator it=request.headerFields.begin(); it != request.headerFields.end(); ++it)
	{
		out << it->first << ": "
		<< it->second << "\n";
	}
	out << request.getBody() << std::endl;
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