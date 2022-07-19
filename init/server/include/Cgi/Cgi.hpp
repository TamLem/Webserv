#ifndef __CGI_HPP__
#define __CGI_HPP__

#include "Server.hpp"

using std::string;
using std::map;
using std::vector;

class Cgi
{
private:
	char **_env;
public:
	Cgi(Request &request)
	{
		const std::map<std::string, std::string>& reqHeaders = request.getHeaderFields();
		
		string envVars[] = {"CONTENT_TYPE", "GATEWAY_INTERFACE", "PATH_INFO", "PATH_TRANSLATED", 
			"QUERY_STRING", "REMOTE_ADDR", "REMOTE_HOST", "REQUEST_METHOD", "SCRIPT_NAME", 
			"SERVER_NAME", "SERVER_PORT", "SERVER_PROTOCOL", };
	
		reqHeaders.find("Accept");
		reqHeaders.find("Content-Type");
		reqHeaders.find("Host");
		reqHeaders.find("Referer");
		reqHeaders.find("Content-Length");
		reqHeaders.find("Origin"); // client address, POST only
		reqHeaders.find("Referer"); //client page where request was made from
		reqHeaders.find("Content-Length");
		reqHeaders.find("REMOTE_ADDR");
		reqHeaders.find("REMOTE_HOST"); //if not provided same as REMOTE_ADDR
		reqHeaders.find("REQUEST_METHOD"); //case-sensitive GET, POST, UPDATE
		reqHeaders.find("SCRIPT_NAME");
		reqHeaders.find("SERVER_NAME");













	}

	~Cgi();

	void setEnv(Request &request) const;
};

void cgi_response(std::string buffer, int fd);

#endif