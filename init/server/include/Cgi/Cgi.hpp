#ifndef __CGI_HPP__
#define __CGI_HPP__

#include "Base.hpp"
#include "Request.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

using std::string;
using std::map;
using std::vector;
using std::cout;
using std::endl;

#define ENV_LEN 14

class Cgi
{
	private:
		char** _env;
		string _method;
		string _scriptName;
		string _pathInfo;
		string _queryString;
	public:
		Cgi(Request &request)
		{
			_env = NULL;
			_method = request.getMethod();

			string url = request.getUrl();
			int scriptNameStart = url.find("/cgi/") + 5;
			int scriptNameEnd = url.find("/", scriptNameStart);
			if (scriptNameEnd == (int)string::npos)
				scriptNameEnd = url.find("?", scriptNameStart);
			int queryStart = url.find("?");
			_scriptName = url.substr(scriptNameStart, scriptNameEnd - scriptNameStart);
			_pathInfo = (scriptNameEnd != (int)string::npos ) ?  url.substr(scriptNameEnd + 1, queryStart - scriptNameEnd - 1) : "";
			_queryString = (queryStart != (int)string::npos )? url.substr(queryStart + 1, string::npos) : "";
			setEnv(request);

			// "Accept");
			// "Host");
			// "Referer");
			// "Content-Length");
			// "Origin"); // client address, POST only
			// "Referer"); //client page where request was made from
			// "Content-Length");
			// "REMOTE_ADDR"); //refere
			// "REMOTE_HOST"); //if not provided same as REMOTE_ADDR
			// "REQUEST_METHOD"); //case-sensitive GET, POST, UPDATE
			// "SCRIPT_NAME");
			// "SERVER_NAME");
		}

		void printEnv()
		{
			for (int i=0; _env[i]; i++)
				std::cout << _env[i] << std::endl;
		}

		~Cgi()
		{
			
		}

	void setEnv(Request &request);
	void cgi_response(std::string buffer, int fd);
};


#endif