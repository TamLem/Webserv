#include "Response.hpp"

#include <string> //std::string
#include <map> //std::map
#include <sstream> //std::stringstream
#include <iostream> //std::ios
#include <fstream> //std::ifstream
#include <sys/socket.h> // send
#include <unistd.h>

#define RESET "\033[0m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RED "\033[31m"

bool Response::isValidStatus(const int status)
{
	if (this->messageMap.count(status))
		return (true);
	return (false);
}

Response::Response(std::string protocol, int status, int fd, std::string url) : fd(fd), url(url)
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
	if (this->url == DEFAULT_URI)
		this->createErrorBody();
	else
		this->createBody();
	this->createHeaderFields();
	this->sendResponse();
}

Response::~Response(void)
{

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

	stream << this->protocol << " " << this->status << " " << this->statusMessage << CRLF;
	for (std::map<std::string, std::string>::const_iterator it = this->headerFields.begin(); it != this->headerFields.end(); ++it)
	{
		stream << it->first << ": "
		<< it->second << CRLF;
	}
	stream << CRLF;
	return (stream.str());
}

int Response::sendall(const int sock_fd, char *buffer, const int len) const
{
	int total;
	int bytesleft;
	int n;
	// int i = 0;

	total = len;
	bytesleft = len;
	while (total > 0)
	{
		n = send(sock_fd, buffer, bytesleft, 0);
		if (n == -1)
		{
			perror("send");
			return (-1);
		}
		total -= n;
		bytesleft -= n;
		buffer += n;
	}
	close(this->fd);
	return (0);
}

void Response::createErrorBody(void)
{
	std::stringstream body;
	body <<
	"<html>\n\
	<head>\n\
	<title>Error " << this->status << "</title>\n\
	</head>\n\
	<body bgcolor=\"000000\">\n\
	<center>\n\
	<h1 style=\"color:white\">Error " << this->status << "</h1>\n\
	<p style=\"color:white\">" << this->statusMessage << "!\n\
	<br><br>\n\
	<img src=\"images/error.jpg\" align=\"TOP\">\n\
	</center>\n\
	</body>\n\
	</html>";
	this->body = body.str();
}

void Response::createBody(void)
{
	std::stringstream body;
	std::ifstream file(this->url.c_str(), std::ios::binary);
	if (file.is_open())
	{
		body << file.rdbuf();
		file.close();
		this->body = body.str();
	}
	else
	{
		perror(NULL);
		throw ERROR_404();
		//404 response
	}
}

void Response::createHeaderFields(void)
{
	std::stringstream contentLength;
	addHeaderField("Server", "localhost:8080");
	if (headerFields.count("Transfer-Encoding") == 0)
	{
		contentLength << this->body.length();
		addHeaderField("Content-Length", contentLength.str());
	}
}

void Response::sendResponse(void)
{
	std::string message = this->constructHeader() + this->body;
	sendall(this->fd, (char *)message.c_str(), message.length());
}

void Response::createMessageMap(void)
{
	//1xx informational response
	this->messageMap[100] = "Continue";
	//2xx success
	this->messageMap[200] = "OK";
	//3xx redirection
	this->messageMap[300] = "Multiple Choices";
	//4xx client errors
	this->messageMap[400] = "Bad Request";
	this->messageMap[401] = "Unauthorized";
	this->messageMap[403] = "Forbidden";
	this->messageMap[404] = "Not Found";
	//5xx server errors
	this->messageMap[500] = "Internal Server Error";
}

std::ostream& operator<<(std::ostream& out, const Response& response)
{
	out << response.getProtocol() << " "
	<< response.getStatus() << " "
	<< response.getStatusMessage();
	return (out);
}

const char* Response::InvalidStatus::what() const throw()
{
	return ("Exception: invalid status");
}

const char* Response::ERROR_404::what() const throw()
{
	return ("Exception: 404");
}