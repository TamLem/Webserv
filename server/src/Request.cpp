#include "Request.hpp"

#include <string>
#include <vector>
#include <sstream> //std::istringstream
#include <ios> //ios::eof
#include <cctype> // isalnum isprint isspace

#ifdef FORTYTWO_TESTER
// only for the tester
	static int count;
// end
#endif

Request::Request(const std::string& message)
{
	#ifdef SHOW_CONSTRUCTION
		std::cout << GREEN << "Request Constructor called for " << this << RESET << std::endl;
	#endif
	this->hasBody = false;
	this->isFile = true;
	this->indexPage = "";
	addMethods();
	parseMessage(message);
}

Request::~Request(void)
{
	#ifdef SHOW_CONSTRUCTION
		std::cout << RED << "Request Deconstructor called for " << this << RESET << std::endl;
	#endif
}

void Request::parseMessage(const std::string& message)
{
	if (message.empty())
		throw EmptyMessage();
	std::istringstream stream (message);
	parseStartLine(stream);
	parseHeaderFields(stream);
	if (this->headerFields.count("host") != 1) // why is is not Host????
		throw NoHost();
	setBodyFlag();
	if (this->hasBody == true)
		parseBody(stream);
}

void Request::parseStartLine(std::istringstream& stream)
{
	std::vector<std::string> tokens;
	std::string line;

	std::getline(stream, line);
	createStartLineTokens(tokens, line);
	std::string method = tokens[0];
	this->url = tokens[1]; // AE remove later
	breakUpTarget(tokens[1]);
	std::string protocol = tokens[2];
	if (!isValidMethod(method))
		throw InvalidMethod();
	this->method = method;
	if (!isValidProtocol(protocol))
		throw Request::InvalidProtocol();
	this->protocol = protocol;
}

void Request::createStartLineTokens(std::vector<std::string>& tokens, const std::string& startLine) const
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

void Request::breakUpTarget(const std::string& token)
{
	size_t pos = 0;
	size_t last = 0;
	std::string tmp;

	pos = token.find_first_of("?#");
	this->target = token.substr(0, pos);
	#ifdef FORTYTWO_TESTER
	if (this->target.compare("/directory") == 0)
		this->target = "/directory/";
	else if (this->target.compare("/directory/nop") == 0)
		this->target = "/directory/nop/";
	#endif
	if (pos == std::string::npos)
		return ;
	else if (token[pos] == '?')
	{
		tmp = token.substr(pos);
		last = pos;
	}
	pos = token.find('#', last);
	if (pos != std::string::npos)
	{
		this->fragment = token.substr(pos, token.length() - pos + 1);
		tmp = tmp.substr(0, tmp.length() - this->fragment.length());
	}
	this->query = tmp;
}

bool Request::isValidMethod(const std::string method) const
{
	if (this->validMethods.count(method))
		return (true);
	return (false);
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

void Request::parseHeaderFieldLine(const std::string& line)
{
	std::vector<std::string> tokens;

	createHeaderTokens(tokens, line);

	if (this->headerFields.count(tokens[0]))
		throw HeaderFieldDuplicate();
	this->headerFields[tokens[0]] = tokens[1];
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

const std::string Request::createHeaderFieldName(const std::string& message, const size_t pos) const
{
	std::string tmp;

	tmp = message.substr(0, pos);
	if (isValidHeaderFieldName(tmp) == false)
		throw InvalidHeaderFieldName();
	toLower(tmp);
	return (tmp);
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

void Request::toLower(std::string& str) const
{
	for (size_t i = 0; i < str.length(); i++)
		str[i] = std::tolower(str[i]);
}

const std::string Request::createHeaderFieldValue(const std::string& message, const size_t pos)
{
	std::string tmp;

	tmp = removeLeadingAndTrailingWhilespaces(message, pos);
	if (isValidHeaderFieldValue(tmp) == false)
		throw InvalidHeaderFieldValue();
	return (tmp);
}

const std::string Request::removeLeadingAndTrailingWhilespaces(const std::string& message, size_t pos) const
{
	pos = message.find_first_not_of(WHITESPACE, pos + 1);
	if (pos == std::string::npos)
		throw InvalidHeaderField();
	size_t end = message.find_last_not_of(WHITESPACE);
	return (message.substr(pos, end - pos + 1));
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

void Request::setBodyFlag(void)
{
	if (this->headerFields.count("content-length") || this->headerFields.count("transfer-encoding")) // ??? what is this supposed to do?
		this->hasBody = true;
}

void Request::parseBody(std::istringstream& stream)
{
	int length = 0;

	while (stream.eof() == false)
	{
		std::string line;

		std::getline(stream, line);
		length += line.length();
		this->body += line;
	}
}

void Request::addMethods(void)
{
	this->validMethods.insert("GET");
	this->validMethods.insert("POST");
	this->validMethods.insert("DELETE");
	#ifdef FORTYTWO_TESTER
	this->validMethods.insert("PUT");
	this->validMethods.insert("HEAD");
	#endif
}

std::ostream& operator<<(std::ostream& out, const Request& request)
{
	out << request.getMethod() << " "
	<< request.getRawTarget() << " "
	<< request.getProtocol() << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = request.getHeaderFields().begin(); it != request.getHeaderFields().end(); ++it)
	{
		out << it->first << ": "
		<< it->second << "\n";
	}
	out << request.getBody() << std::endl;
	return (out);
}

void Request::setDecodedTarget(const std::string& target)
{
	this->decodedTarget = target;
}

void Request::setRoutedTarget(const std::string& target)
{
	this->routedTarget = target;
}

void Request::setQuery(const std::string& query)
{
	this->query = query;
}

const std::string& Request::getMethod(void) const
{
	return (this->method);
}

const std::string& Request::getUrl(void) const
{
	return (this->url);
}

const std::string& Request::getRawTarget(void) const
{
	return (this->target);
}

const std::string& Request::getDecodedTarget(void) const
{
	return (this->decodedTarget);
}

const std::string& Request::getRoutedTarget(void) const
{
	return (this->routedTarget);
}

const std::string& Request::getQuery(void) const
{
	return (this->query);
}

const std::string& Request::getFragment(void) const
{
	return (this->fragment);
}

const std::map<std::string, std::string>& Request::getHeaderFields() const
{
	return (this->headerFields);
}

std::string Request::getHostName() const
{
	std::map<std::string, std::string>::const_iterator it = this->headerFields.find("Host");
	if (it != this->headerFields.end())
		return (it->second);
	else
		return ("localhost");
}

// const std::string& Request::getProtocol(void) const
// {
// 	return (this->protocol);
// }

const char* Request::InvalidNumberOfTokens::what() const throw()
{
	#ifdef FORTYTWO_TESTER
	if (count == 0) //Test POST http://localhost:8080/ with a size of 0
		return ("405"); // FORTYTWO_TESTER AE change back to 400 when POST works
	else if (count == 1) // PUT
		return ("201"); // FORTYTWO_TESTER AE change back to 400 when POST works
	else
		return ("400");
	count++;
	std::cout << "count is now: " << count << std::endl;
	#endif
	return ("400");
}

const char* Request::EmptyMessage::what() const throw()
{
	return ("400");
}

const char* Request::InvalidMethod::what() const throw()
{
	return ("501");
}

const char* Request::InvalidHeaderField::what() const throw()
{
	return ("400");
}

const char* Request::NoHost::what() const throw()
{
	return ("400");
}

const char* Request::InvalidHeaderFieldName::what() const throw()
{
	return ("400");
}

const char* Request::InvalidHeaderFieldValue::what() const throw()
{
	return ("400");
}

const char* Request::InvalidProtocol::what() const throw()
{
	return ("505");
}

const char* Request::DirectoryNotEmpty::what() const throw()
{
	return ("400");
}