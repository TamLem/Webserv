#include "Cgi/Cgi.hpp"

#include <string.h>

Cgi::Cgi(Request &request, ConfigStruct configStruct): _scriptExists(true), _confStruct(configStruct)
{
	#ifdef SHOW_CONSTRUCTION
		std::cout << GREEN << "Cgi Constructor called for " << this << RESET << std::endl;
	#endif

	_docRoot = configStruct.root;

	_method = request.getMethod();
	string url = request.getDecodedTarget();
	string extension = url.substr(url.find_last_of("."));
	cout << "extension: " << extension << endl;

	if (url.find("/cgi/") == string::npos)
		_scriptExists = false;
	
	int queryStart = url.find("?");
	if (_scriptExists)
	{
		int scriptNameStart = url.find("/cgi/") + 5;
		int scriptNameEnd = url.find("/", scriptNameStart);
		if (scriptNameEnd == (int)string::npos)
			scriptNameEnd = url.find("?", scriptNameStart);
		_scriptName = url.substr(scriptNameStart, scriptNameEnd - scriptNameStart);
		_scriptName = "." + _docRoot + "cgi-bin/" + _scriptName;
		_pathInfo = (scriptNameEnd != (int)string::npos ) ? 
			url.substr(scriptNameEnd + 1, queryStart - scriptNameEnd - 1) : "";	
	}
	else
	{
		if (_confStruct.cgi.count(extension) != 0)
		{
			_scriptName = _confStruct.cgi[extension];
		}
		_pathInfo = url;
		_pathTranslated = "." + _docRoot + _pathInfo.substr(1);	
	}
	_queryString = request.getQuery().substr(1);
	setEnv(request);
}

Cgi::~Cgi()
{
	#ifdef SHOW_CONSTRUCTION
		std::cout << RED << "Cgi Deconstructor called for " << this << RESET << std::endl;
	#endif
}

void Cgi::setEnv(Request &request) // think about changing this to return a const char **
{
	std::map<std::string, std::string> reqHeaders = request.getHeaderFields();

	_env["REQUEST_METHOD"] = _method;
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["PATH_INFO"] = _pathInfo;
	_env["PATH_TRANSLATED"] = _pathTranslated;
	_env["SCRIPT_NAME"] = _scriptName;
	_env["QUERY_STRING"] = _queryString;
	_env["SERVER_NAME"] = reqHeaders.find("host")->second.substr(0, reqHeaders.find("host")->second.find(":"));
	_env["SERVER_PORT"] =  reqHeaders.find("host")->second.substr(reqHeaders.find("host")->second.find(":") + 1);
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	if (_method == "POST")
	{
		_env["CONTENT_TYPE"] = reqHeaders["Content-Type"];
		_env["CONTENT_LENGTH"] = reqHeaders["Content-Length"];
		_env["REMOTE_HOST"] = reqHeaders["Host"];
		_env["REMOTE_ADDR"] = reqHeaders["X-Forwarded-For"];
	}
	_env["REDIRECT_STATUS"] = "200";

}

static char **mapToStringArray(const std::map<std::string, std::string> &map)
{
	char **env = new char*[map.size() + 1];
	int i = 0;
	for (std::map<string, string>::const_iterator it = map.begin(); it != map.end(); ++it)
	{
		env[i] = strdup((it->first + "=" + it->second).c_str());
		i++;
	}
	env[i] = NULL;
	return env;
}

void Cgi::printEnv()
{
	cout << "Environment variables:" << endl;
	for (std::map<string, string>::const_iterator it = _env.begin(); it != _env.end(); ++it)
	{
		cout << it->first << ": " << it->second << endl;
	}
}

void Cgi::cgi_response(int fd)
{
	std::string file;
	std::string executable;
	char **args;
	int pipefd[2];

	args = NULL;
	executable = _scriptName;
	std::cout << GREEN << "Executing CGI..." << std::endl;
	file = _pathInfo;
	int stdout_init = dup(STDOUT_FILENO);
	pipe(pipefd);
	dup2(pipefd[1], STDOUT_FILENO);
	close(pipefd[1]);
	int pid = fork();
	if (pid == 0)
	{
		cout << "executing " << executable << endl;
		if (execve(executable.c_str(), args, mapToStringArray(_env)) == -1)
		{
			std::cerr << "error executing cgi" << std::endl;
		}
		exit(0);
	}
	wait(NULL);
	dup2(stdout_init, STDOUT_FILENO);
	std::string header = "HTTP/1.1 200 OK\r\n"; //cgi status code
	send(fd, header.c_str(), header.size(), 0);
	char buf[1024];
	buf[1023] = '\0';
	int n;
	while((n = read(pipefd[0], buf, 1023)) > 0)
	{
		cout << "cgi output: " << buf << endl;
		send(fd, buf, n, 0);
	}
	close(pipefd[0]);
	// close(fd);
}