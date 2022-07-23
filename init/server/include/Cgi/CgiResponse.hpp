#ifndef __CGI_RESPONSE_HPP__
# define __CGI_RESPONSE_HPP__
 
#include "Response.hpp"

class CgiResponse: public Response
{
private:
	/* data */
public:
	CgiResponse(/* args */);
	~CgiResponse();

	void parseCgiHeaders(std::string buf);
};

CgiResponse::CgiResponse(/* args */)
{
}

CgiResponse::~CgiResponse()
{
}

void CgiResponse::parseCgiHeaders(std::string buf)
{

}

#endif