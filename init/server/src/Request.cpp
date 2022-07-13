#include "Request.hpp"

#include <string>
#include <vector>
#include <sstream> //std::istringstream
#include <ios> //ios::eof
#include <cctype> // isalnum isprint isspace

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

void Request::createStartLineTokens(std::vector<std::string>& tokens, const std::string& startLine)
{
	size_t last = 0;
	size_t next = 0;
	size_t count = 3;

	if (*startLine.rbegin() != CR)
		throw InvalidStartLine();
	while ((next = startLine.find(SP, last)) != std::string::npos && tokens.size() < count + 1)
	{
		tokens.push_back(startLine.substr(last, next - last));
		last = next + 1;
	}
	next = startLine.find_last_of(CR);
	tokens.push_back(startLine.substr(last, next - last));
	if (tokens.size() != count)
		throw InvalidNumberOfTokens();
}

bool Request::isValidHeaderFieldName(const std::string& token) const
{
	std::string tchars = TCHAR;

	for (size_t i = 0; i < token.length(); i++)
	{
		char c = token[i];
		if (isalnum(c) == false && tchars.find(c) == std::string::npos)
			return (false);
	}
	return (true);
}

bool Request::isValidHeaderFieldValue(const std::string& token) const
{
	for (size_t i = 0; i < token.length(); i++)
	{
		char c = token[i];
		if (isprint(c) == false)
			return (false);
	}
	return (true);
}

void Request::toLower(std::string& str)
{
	for (size_t i = 0; i < str.length(); i++)
		str[i] = std::tolower(str[i]);
}

const std::string Request::removeLeadingAndTrailingWhilespaces(const std::string& message, size_t pos)
{
	pos = message.find_first_not_of(WHITESPACES, pos + 1);
	if (pos == std::string::npos)
		throw InvalidHeaderField();
	size_t end = message.find_last_not_of(WHITESPACES);
	return (message.substr(pos, end - pos + 1));
}

const std::string Request::createHeaderFieldName(const std::string& message, size_t pos)
{
	std::string tmp;
	
	tmp = message.substr(0, pos);
	if (isValidHeaderFieldName(tmp) == false)
		throw InvalidHeaderFieldName();
	toLower(tmp);
	return (tmp);
}

const std::string Request::createHeaderFieldValue(const std::string& message, size_t pos)
{
	std::string tmp;
	
	tmp = removeLeadingAndTrailingWhilespaces(message, pos);
	if (isValidHeaderFieldValue(tmp) == false)
		throw InvalidHeaderFieldValue();
	return (tmp);
}

void Request::createHeaderTokens(std::vector<std::string>& tokens, const std::string& headerField)
{
	size_t pos = 0;
	std::string tmp;

	if (*headerField.rbegin() != CR)
		throw InvalidHeaderField();
	pos = headerField.find(':');
	if (pos == std::string::npos)
		throw InvalidHeaderField();
	tmp = createHeaderFieldName(headerField, pos);
	tokens.push_back(tmp);
	tmp = createHeaderFieldValue(headerField, pos);
	tokens.push_back(tmp);
}

//AE check what happens for space before method and multiple spaces or 
void Request::parseStartLine(std::istringstream& stream)
{
	std::vector<std::string> tokens;
	std::string line;

	std::getline(stream, line);
	createStartLineTokens(tokens, line);
	std::string method = tokens[0];
	std::string url = tokens[1];
	std::string protocol = tokens[2];
	if (!isValidMethod(method))
		throw InvalidMethod();
	if (!isValidProtocol(protocol))
		throw InvalidProtocol();
	this->method = method;
	if (url == "/")
		this->url = INDEX_PATH;
	else
		this->url = "." + url;
	this->protocol = protocol;
}

void Request::parseHeaderFieldLine(const std::string& line)
{
	std::vector<std::string> tokens;

	createHeaderTokens(tokens, line);
	
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
		if (line.length() == 1 && *line.begin() == CR)
			break ;
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
	if (this->headerFields.count("content-length") || this->headerFields.count("transfer-encoding")) //AE handle caseinsensitivity
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
	for (std::map<std::string, std::string>::const_iterator it = request.getHeaderFields().begin(); it != request.getHeaderFields().end(); ++it)
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

const char* Request::InvalidHeaderField::what() const throw()
{
	return ("Exception: invalid header field");
}

const char* Request::InvalidHeaderFieldName::what() const throw()
{
	return ("Exception: detected invalid character in http message header field-name");
}

const char* Request::InvalidHeaderFieldValue::what() const throw()
{
	return ("Exception: detected invalid character in http message header field-value");
}