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

string &CgiResponse::getBody()
{
	string line = readline(_cgiFD);
	while (line != "\r")
		line = readline(_cgiFD);
	//save the rest in body
	char buf[1024];
	int n = 0;
	while ((n = read(_cgiFD, buf, 1023)) > 0)
	{
		buf[n] = '\0';
		// cout << "buf: " << buf << endl;
		this->_body.append(buf, n);
	}
	return (_body);
}

// void CgiResponse::sendResponse() // AE @tam is this still in use?
// {
// 	cout << "sending response" << " body" << this->_body << endl;
// 	string headers  = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(this->_body.length()) + "\r\n\r\n"; // @Tam please do not send a hardcoded response, what if the cgi crahed or something went wrong?
// 	string resp = headers + this->_body; // is it really needed to create a full response with headers here????
// 	send(_clientFD, resp.c_str(), resp.length(), 0); // @Tam please do not send anything directly to the client since it is strictly forbidden by the subject
// 	#ifdef SHOW_LOG_CGI
// 		LOG_GREEN(resp);
// 	#endif

// 	// send(_cgiFD, headers.c_str(), headers.length(), 0);
// 	// tunnelResponse(_cgiFD, _clientFD);
// }

void CgiResponse::tunnelResponse(int srcFD, int destFD)
{
	cout << "tunneling response" << endl;
	int n;
	char buf[1025];

	buf[1024] = '\0';
	while ((n = read(srcFD, buf, 1024)) > 0)
	{
		send(destFD, buf, n, 0); // @Tam please do not send anything directly to the client since it is strictly forbidden by the subject
	}
	close(this->_cgiFD); // @Tam is this the closing of the tempfile? because you need to close it once you are done reading all the data from it
	close(this->_clientFD); // @Tam never close the clientFD anywhere, let it happen in the mainloop after the response was sent
}


string CgiResponse::getValue(std::string &keyValueString)
{
	string value;

	value = keyValueString.substr(keyValueString.find(":") + 1, string::npos);
	return value;
}
