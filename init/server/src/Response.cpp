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

Response::Response(int status, int fd, std::string uri) : fd(fd), uri(uri)
{
	this->createMessageMap();
	if (!isValidStatus(status))
		throw InvalidStatus();
	this->protocol = PROTOCOL;
	this->status = status;
	this->statusMessage = this->messageMap[this->status];
	this->hasBody = true;
	this->createBody();
	this->createHeaderFields();
	this->sendResponse();
}

Response::Response(int status, int fd) : fd(fd)
{
	this->createMessageMap();
	if (!isValidStatus(status))
		throw InvalidStatus();
	this->protocol = PROTOCOL;
	this->status = status;
	this->statusMessage = this->messageMap[this->status];
	this->hasBody = true;
	this->createErrorBody();
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
	<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"images/favicon.ico\">\n\
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
	std::ifstream file(this->uri.c_str(), std::ios::binary);
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
	this->messageMap[101] = "Switching Protocols";
	this->messageMap[102] = "Processing";
	this->messageMap[103] = "Early Hints";
	//2xx success
	this->messageMap[200] = "OK";
	this->messageMap[201] = "Created";
	this->messageMap[202] = "Accepted";
	this->messageMap[203] = "Non-Authoritative Information";
	this->messageMap[204] = "No Content";
	this->messageMap[205] = "Reset Content";
	this->messageMap[206] = "Partial Content";
	this->messageMap[207] = "Multi-Status";
	this->messageMap[208] = "Already Reported";
	this->messageMap[226] = "IM Used";
	//3xx redirection
	this->messageMap[300] = "Multiple Choices";
	this->messageMap[301] = "Moved Permanently";
	this->messageMap[302] = "Found";
	this->messageMap[303] = "See Other";
	this->messageMap[304] = "Not Modified";
	this->messageMap[305] = "Use Proxy";
	this->messageMap[306] = "Switch Proxy";
	this->messageMap[307] = "Temporary Redirect";
	this->messageMap[308] = "Permanent Redirect";
	//4xx client errors
	this->messageMap[400] = "Bad Request";
	this->messageMap[401] = "Unauthorized";
	this->messageMap[402] = "Payment Required";
	this->messageMap[403] = "Forbidden";
	this->messageMap[404] = "Not Found";
	this->messageMap[405] = "Method Not Allowed";
	this->messageMap[406] = "Not Acceptable";
	this->messageMap[407] = "Proxy Authentication Required";
	this->messageMap[408] = "Request Timeout";
	this->messageMap[409] = "Conflict";
	this->messageMap[410] = "Gone";
	this->messageMap[411] = "Length Required";
	this->messageMap[412] = "Precondition Failed";
	this->messageMap[413] = "Content Too Large";
	this->messageMap[414] = "URI Too Long";
	this->messageMap[415] = "Unsupported Media Type";
	this->messageMap[416] = "Range Not Satisfiable";
	this->messageMap[417] = "Expectation Failed";
	this->messageMap[418] = "I'm a teapot";
	this->messageMap[421] = "Misdirected Request";
	this->messageMap[422] = "Unprocessable Content";
	this->messageMap[423] = "Locked";
	this->messageMap[424] = "Failed Dependency";
	this->messageMap[425] = "Too Early";
	this->messageMap[426] = "Upgrade Required";
	this->messageMap[428] = "Precondition Required";
	this->messageMap[429] = "Too Many Requests";
	this->messageMap[431] = "Request Header Fields Too Large";
	this->messageMap[451] = "Unavailable For Legal Reasons";
	//5xx server errors
	this->messageMap[500] = "Internal Server Error";
	this->messageMap[501] = "Not Implemented";
	this->messageMap[502] = "Bad Gateway";
	this->messageMap[503] = "Service Unavailable";
	this->messageMap[504] = "Gateway Timeout";
	this->messageMap[505] = "HTTP Version Not Supported";
	this->messageMap[506] = "Variant Also Negotiates";
	this->messageMap[507] = "Insufficient Storage";
	this->messageMap[508] = "Loop Detected";
	this->messageMap[510] = "Not Extended";
	this->messageMap[511] = "Network Authentication Required";
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