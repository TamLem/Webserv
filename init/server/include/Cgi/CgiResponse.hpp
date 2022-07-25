#ifndef __CGI_RESPONSE_HPP__
# define __CGI_RESPONSE_HPP__
 
#include "Response.hpp"
#include "Cgi/Cgi.hpp"

class CgiResponse: public Response
{
private:
	int	_responseFd;
public:
	CgiResponse(int fd);
	~CgiResponse();

	void parseCgiHeaders(std::string buf);
	string& readline(int fd);
	bool checkForMandatoryHeaders(string& headerLine);
};

CgiResponse::CgiResponse(int fd): _responseFd(fd)
{
	
}

CgiResponse::~CgiResponse()
{
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


void CgiResponse::parseCgiHeaders(std::string buf)
{
	string line = readline(_responseFd);
	if (!checkForMandatoryHeaders(line))
		throw ERROR_404();
	if (line.find("Location") != string::npos)
	{
		if (line.find("http://") || line.find("localhost")) //treat as absolute request
		{

		}
		else
		{
			//relative redirect url
		}
	}
	if (line.find("Status") != string::npos)
	{

	}
	else
	{
	}
	if (line.find("Content-Type") != string::npos)
	{
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


#endif