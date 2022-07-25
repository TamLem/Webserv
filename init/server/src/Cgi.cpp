#include "Cgi/Cgi.hpp"

#include <string.h>

Cgi::Cgi(Request &request)
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
	_pathInfo = (scriptNameEnd != (int)string::npos ) ? url.substr(scriptNameEnd + 1, queryStart - scriptNameEnd - 1) : "";
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

Cgi::~Cgi()
{
	int i = 0;
	while(_env[i])
		delete _env[i++];
	delete _env;
}

void Cgi::setEnv(Request &request) // think about changing this to return a const char **
{
	const std::map<std::string, std::string>& reqHeaders = request.getHeaderFields();
	string envVars[] = { "REQUEST_METHOD", "GATEWAY_INTERFACE", "PATH_INFO", "PATH_TRANSLATED", "SCRIPT_NAME",
		"QUERY_STRING", "SERVER_NAME", "SERVER_PORT", "SERVER_PROTOCOL", "CONTENT_TYPE", "CONTENT_LENGTH", "REMOTE_HOST", "REMOTE_ADDR", };

	_env = new char*[ENV_LEN];
	_env[ENV_LEN - 1] = NULL;

	int i = 0;
	_env[i] = strdup((envVars[i] + "=" + _method).c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + "CGI/1.1").c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + _pathInfo).c_str()); i++;
	_env[i] = _pathInfo.empty() ? strdup((envVars[i] + "=" + "").c_str()) : strdup((envVars[i] + "=" + "./" + _pathInfo).c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + "./" + _scriptName).c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + _queryString).c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + "localhost").c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + "8080").c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + "HTTP/1.1").c_str()); i++;
	if (_method == "POST")
	{
		_env[i] = strdup((envVars[i] + "=" + reqHeaders.find("Content-Type")->second).c_str()); i++;
		_env[i] = strdup((envVars[i] + "=" + reqHeaders.find("Content-Length")->second).c_str()); i++;
		_env[i] = strdup((envVars[i] + "=" + reqHeaders.find("Origin")->second).c_str()); i++;
		_env[i] = strdup((envVars[i] + "=" + reqHeaders.find("Origin")->second).c_str()); i++;
	}

	while(_env[i])
		_env[i++] = NULL;
}

void Cgi::cgi_response(std::string buffer, int fd)
{
	std::string file;
	int pipefd[2];

	(void)buffer;
	std::cout << GREEN << "Executing CGI..." << std::endl;
	file = "index.php";
	int stdout_init = dup(STDOUT_FILENO);
	pipe(pipefd);
	dup2(pipefd[1], STDOUT_FILENO);
	close(pipefd[1]);
	int pid = fork();
	if (pid == 0)
	{
		// if(execlp("/usr/bin/php", "php", file.c_str(), NULL) == -1)
		chdir("cgi-bin");
		if (execve(_scriptName.c_str() , NULL, _env) == -1)
		{
			std::cout << "error executing cgi" << std::endl;
		}
		exit(0);
	}
	wait(NULL);
	dup2(stdout_init, STDOUT_FILENO);
	std::string header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
	send(fd, header.c_str(), header.size(), 0);
	char buf[1024];
	int n;
	while((n = read(pipefd[0], buf, 1024)))
	{
		send(fd, buf, n, 0);
	}
	close(pipefd[0]);
	close(fd);
}
