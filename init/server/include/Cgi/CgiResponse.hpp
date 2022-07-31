#ifndef __CGI_RESPONSE_HPP__
# define __CGI_RESPONSE_HPP__
 
#include "Response.hpp"
#include "Cgi/Cgi.hpp"
#include "Cgi/CgiResponse.hpp"

class CgiResponse: public Response
{
private:
	int	_cgiFD;
	int _clientFD;

public:
	CgiResponse(int cgiFD, int clientFD);
	~CgiResponse();

	void parseCgiHeaders(std::string buf);
	string& readline(int fd);
	bool checkForMandatoryHeaders(string& headerLine);
	void parseSingleHeaderField(string& headerLine);
	string &getValue(std::string &keyValueString);
	void sendResponse();
	void tunnelResponse(int cgiFD, int clientFD);

};



#endif