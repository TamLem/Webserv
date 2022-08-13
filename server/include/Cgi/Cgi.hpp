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
	// defines only to not have undefined behaviour
		Cgi();
		Cgi(const Cgi&);
		Cgi& operator=(const Cgi&);

	// private variables
		string _docRoot;
		std::map<string, string> _env;
		string _method;
		string _scriptName;
		string _pathInfo;
		string _pathTranslated;
		string _queryString;
		bool	_selfExecuting;
		ConfigStruct _confStruct;
	public:
		Cgi(Request &request, ConfigStruct confStruct);

		~Cgi();

		void setEnv(Request &request);
		void cgi_response(int fd);
		void phpHandler(Request &req);
		void passAsInput();
		void printEnv();
};
#endif