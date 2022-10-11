#include "Cgi/CgiResponse.hpp"

CgiResponse::CgiResponse(int cgiFD, int clientFD): _cgiFD(cgiFD), _clientFD(clientFD) // undefined fd???? Tam please check this
{
	#ifdef SHOW_CONSTRUCTION
		std::cout << GREEN << "CgiResponse Constructor called for " << this << RESET << std::endl;
	#endif
}

CgiResponse::~CgiResponse()
{
	#ifdef SHOW_CONSTRUCTION
		std::cout << RED << "CgiResponse Deconstructor called for " << this << RESET << std::endl;
	#endif
}

string CgiResponse::readline(int fd)
{
	char 	buf[2];
	string	line;

	buf[1] = '\0';
	int n = read(fd, buf, 1);
	if (n <= 0)
	{
		#ifdef SHOW_LOG_CGI
			LOG_RED("Error reading from CGI");
		#endif
		throw std::runtime_error("Error reading from CGI");
	}
	while (n > 0 && *buf != '\n')
	{
		line = line + string(buf);
		n = read(fd, buf, 1);
	}
	
	return (line);
}

string &CgiResponse::getBody()
{
	string line = readline(_cgiFD);
	while (line != "\r")
		line = readline(_cgiFD);
	char buf[1024];
	int n = 0;
	while ((n = read(_cgiFD, buf, 1023)) > 0)
	{
		buf[n] = '\0';
		this->_body.append(buf, n);
	}
	return (_body);
}
