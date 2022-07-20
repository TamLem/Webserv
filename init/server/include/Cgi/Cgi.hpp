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

#define ENV_LEN 14

class Cgi
{
	private:
		char** _env;
		string _method;
	public:
		Cgi(Request &request)
		{
			_env = nullptr;
			_method = request.getMethod();
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
	static void cgi_response(std::string buffer, int fd);
};


#endif