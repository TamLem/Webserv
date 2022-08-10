#include "Cgi/CgiResponse.hpp"

CgiResponse::CgiResponse(int cgiFD, int clientFD): _cgiFD(fd), _clientFD(clientFD) // undefined fd???? Tam please check this
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

string& CgiResponse::readline(int fd)
{
	char 	buf[2];
	string	line;

	buf[1] = '\0';
	int n;
	while ((n = read(fd, buf, 1)) && *buf != '\n')
		line = line + string(buf);
}

void CgiResponse::parseSingleHeaderField(string& headerLine)
{
	if (headerLine.find("Location") != string::npos)
	{
		if (headerLine.find("http://") || headerLine.find("localhost")) //treat as absolute request
		{
			this->addHeaderField("Location", getValue(headerLine));
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
		addHeaderField("Content-Type", getValue(headerLine));
	}
}


void CgiResponse::parseCgiHeaders(std::string buf)
{
	string line = readline(_cgiFD);
	if (!checkForMandatoryHeaders(line))
		throw ERROR_404();

	while (line != CRLF && line.empty())
	{
		parseSingleHeaderField(line);
		line =  readline(_cgiFD);
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

void CgiResponse::sendResponse()
{
	string headers = this->constructHeader();
	send(_cgiFD, headers.c_str(), headers.length(), 0);
	tunnelResponse(_cgiFD, _clientFD);
}

void CgiResponse::tunnelResponse(int srcFD, int destFD)
{
	int n;
	char buf[1025];

	buf[1024] = '\0';
	while ((n = read(srcFD, buf, 1024)) > 0)
	{
		send(destFD, buf, n, 0);
	}
	close(this->_cgiFD);
}


string &CgiResponse::getValue(std::string &keyValueString)
{
	string value;

	value = keyValueString.substr(keyValueString.find(":") + 1, string::npos);
	return value;
}

