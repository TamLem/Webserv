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
	int n = 0;
	while ((n = read(fd, buf, 1)) > 0 && *buf != '\n')
		line = line + string(buf);
	return (line);
}

void CgiResponse::parseSingleHeaderField(string& headerLine)
{
	if (headerLine.find("Location") != string::npos)
	{
		if (headerLine.find("http://") || headerLine.find("localhost")) //treat as absolute request
		{
			this->addHeaderField("location", getValue(headerLine));
		}
		else
		{
			//relative redirect url, handled by server
		}
	}
	if (headerLine.find("Status") != string::npos)
	{
		this->setStatus(getValue(headerLine));
	}
	else
	{
	}
	if (headerLine.find("Content-Type") != string::npos)
	{
		addHeaderField("content-type", getValue(headerLine));
	}
}

bool CgiResponse::checkForMandatoryHeaders(string& headerLine)
{
	string mandatoryHeaders[3] = {"Content-Type", "Location", "Status"};

	for (int i=0; i < 3; i++)
	{
		int pos = headerLine.find(mandatoryHeaders[i]);
		if (pos == 0 && headerLine.length() > mandatoryHeaders[i].length())
			return true;
	}
	return (false);
}

void CgiResponse::parseCgiHeaders()
{
	string line = readline(_cgiFD);
	// if (line.empty() || !checkForMandatoryHeaders(line))
	// 	throw ERROR_404();
	while (line != "\r")
		line = readline(_cgiFD);
	//save the rest in body
	char buf[1024];
	int n = 0;
	while ((n = read(_cgiFD, buf, 1024)) > 0)
	{
		cout << "buf: " << buf << endl;
		this->body.append(buf, n);
	}

	return ; //remove next lines
	while (line != CRLF && !line.empty())
	{
		parseSingleHeaderField(line);
		line =  readline(_cgiFD);
	}

}

void CgiResponse::sendResponse()
{
	cout << "sending response" << endl;
	// string headers = this->constructHeader();
	string headers  = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(this->body.length()) + "\r\n\r\n";
	//print headers
	// cout << headers << endl;
	send(_clientFD, headers.c_str(), headers.length(), 0);
	send(_clientFD, this->body.c_str(), this->body.length(), 0);
	// send(_cgiFD, headers.c_str(), headers.length(), 0);
	// tunnelResponse(_cgiFD, _clientFD);
}

void CgiResponse::tunnelResponse(int srcFD, int destFD)
{
	cout << "tunneling response" << endl;
	int n;
	char buf[1025];

	buf[1024] = '\0';
	while ((n = read(srcFD, buf, 1024)) > 0)
	{
		send(destFD, buf, n, 0);
	}
	close(this->_cgiFD);
	close(this->_clientFD);
}


string CgiResponse::getValue(std::string &keyValueString)
{
	string value;

	value = keyValueString.substr(keyValueString.find(":") + 1, string::npos);
	return value;
}
