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

Response::Response(std::string protocol, int status, int fd, std::string url) : fd(fd), url(url)
{
	this->createMessageMap();
	// std::cout << "MY RESPONSE PROTOCOL:" << protocol << "." << std::endl;
	if (!isValidProtocol(protocol))
		throw InvalidProtocol();
	if (!isValidStatus(status))
		throw InvalidStatus();
	this->protocol = protocol;
	this->status = status;
	this->statusMessage = this->messageMap[this->status];
	this->hasBody = true;
	// this->headerFields = std::make_pair(Content-Type, text/html);
	this->body = "<html><body><h1>Hello World!</h1></body></html>";
	// this->sendResponse();
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

	// std::cerr << BLUE << "old_size " << old_size << RESET << std::endl;
	// stream.clear();
	// stream.str("");
	stream << this->protocol << " " << this->status << " " << this->statusMessage
	// << "\n" <<
	// "Date: " << responseClass.date << "\n" <<
	// << "Content-Type: " << responseClass.type << "\n"
	// << CRLF
	<< CRLF
	<< "Server: localhost:8080"
	<< CRLF
	// << "Content-Type: text/html"
	// << CRLF
	<< "Content-Length: ";
	return (stream.str());
}

size_t Response::ft_intlen(int n)
{
	size_t i;

	i = 0;
	if (n == -2147483648)
		return (10);
	if (n >= 0 && n <= 9)
		return (1);
	if (n < 0)
	{
		n = n * -1;
		i++;
	}
	while (n > 0)
	{
		n = n / 10;
		i++;
	}
	return (i);
}

int Response::sendall(int sock_fd, char *buffer, int len)
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
	return (0);
}

// void Response::readBody(void)
// {

// }

void Response::sendResponse(void)
{
	std::stringstream number_stream;
	std::stringstream body_stream;
	std::ifstream file(this->url.c_str(), std::ios::binary);
	if (file.is_open())
	{
		std::string header_string = this->constructHeader();
		size_t len = header_string.length();
		
		body_stream  << CRLFTWO << file.rdbuf();
		file.close();
		body_stream.seekg(0, std::ios::end);
		len += body_stream.tellg();
		// body_stream.seekg(0, std::ios::beg);
		std::string message;
		do
		{
			number_stream.clear();
			number_stream.str("");
			number_stream << len;
			message = header_string + number_stream.str() + body_stream.str();
			if (len == message.length())
				break ;
			len = message.length();
		} while (true);

		sendall(this->fd, (char *)message.c_str(), message.length());
		std::cerr << RED << len << RESET << std::endl;
	}
	else
	{
		perror(NULL);
		throw ERROR_404();
		//404 response
	}
	close(this->fd);
}

const char* Response::InvalidStatus::what() const throw()
{
	return ("Exception: invalid status");
}

const char* Response::ERROR_404::what() const throw()
{
	return ("Exception: 404");
}