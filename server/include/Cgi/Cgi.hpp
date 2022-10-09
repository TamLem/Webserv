#ifndef __CGI_HPP__
#define __CGI_HPP__

#include "Base.hpp"
#include "Request.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
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
		string _body;
		string _scriptName;
		string _pathInfo;
		string _pathTranslated;
		string _queryString;
		bool	_selfExecuting;
		ConfigStruct _confStruct;
		FILE*  _infile;
	public:
		Cgi(Request &request, ConfigStruct confStruct, FILE *infile);

		~Cgi();

		void setEnv(Request &request);
		void init_cgi(int client_fd, int cgi_out);
		void phpHandler(Request &req);
		void passAsInput();
		void passAsOutput();
		void printEnv();

		class CgiExceptionNotFound : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};
};
#endif