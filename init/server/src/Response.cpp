#include "Response.hpp"

#include <string> //std::string
#include <map> //std::map
#include <sstream> //std::stringstream
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

	stream.clear();
	stream.str("");
	stream << this->protocol << " " << this->status << " " << this->statusMessage
	// << "\n" <<
	// "Date: " << responseClass.date << "\n" <<
	// "Content-Type: " << responseClass.type << "\n" <<
	// "Content-Length: " << responseClass.length
	// << "\r\n\r\n";
	<< "\r\n"
	<< "Server: localhost:8080"
	// << "Content-Type: text/html"
	// << "\r\n"
	// << "Content-Length: 500000"
	<< "\r\n\r\n";
	// << this->body;
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

void Response::sendResponse(void)
{
	// std::cerr << RED << this->url << "." << RESET << std::endl;
	std::ifstream input(this->url, std::ios::binary);
	if (input.is_open())
	{
		std::filebuf *filebuffer = input.rdbuf();
		std::size_t size = filebuffer->pubseekoff(0, input.end, input.in); //filebuffer size
		// cout << size << endl;
		filebuffer->pubseekpos(0, input.in); //reset filebufer pointer to 0
		char *out_buffer = new char[size];
		filebuffer->sgetn(out_buffer, size); //copy filebuffer to outbuffer
		input.close();
		// size += strlen("HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: image/vdn.microsoft.icon\nContent-Length: \n\n") + ft_intlen(size);
		std::string response = 	this->constructHeader();
		size += response.length() + ft_intlen(size);
		// dprintf(fd, "HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: image/vdn.microsoft.icon\nContent-Length: %lu\n\n", size);
		dprintf(this->fd, "%s", response.c_str());
		// write(fd, out_buffer, size);
		sendall(this->fd, out_buffer, size);
		delete[] out_buffer;
	}
	else
	{
		perror(NULL);
		throw ERROR_404();
		//404 response
	}
	
	
	// std::string header = "Content-Type: text/html\r\n\r\n";
	// std::string body = "<html><body><h1>Hello World</h1></body></html>";
	
	// std::string response = 	this->constructHeader();
	// if (write(this->fd, response.c_str(), response.size()) < 0) {
	// 	std::cout << "Error writing to socket" << std::endl;
	// 	close(this->fd);
	// 	// return 1;
	// }
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