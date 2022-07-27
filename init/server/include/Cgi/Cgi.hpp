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
#include <sys/wait.h>

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
		Cgi(Request &request);

		~Cgi();

		void printEnv()
		{
			for (int i=0; _env[i]; i++)
				std::cout << _env[i] << std::endl;
		}


	void setEnv(Request &request);
	void cgi_response(std::string buffer, int fd);
};


#endif