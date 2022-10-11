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
// defines only to not have undefined behaviour
	CgiResponse(const CgiResponse&);
	CgiResponse& operator=(const CgiResponse&);
	CgiResponse(void);

	string _body;

public:
	CgiResponse(int cgiFD, int clientFD);
	~CgiResponse();

	string &getBody();
	string readline(int fd);
};



#endif