#include "Cgi/Cgi.hpp"

void Cgi::setEnv(Request &request)
{
	const std::map<std::string, std::string>& reqHeaders = request.getHeaderFields();
	string envVars[] = { "REQUEST_METHOD", "GATEWAY_INTERFACE", "PATH_INFO", "PATH_TRANSLATED", "SCRIPT_NAME", 
		"QUERY_STRING", "SERVER_NAME", "SERVER_PORT", "SERVER_PROTOCOL", "CONTENT_TYPE", "CONTENT_LENGTH", "REMOTE_HOST", "REMOTE_ADDR", };

	_env = new char*[ENV_LEN];
	_env[ENV_LEN - 1] = nullptr; 
	
	int i = 0;
	_env[i] = strdup((envVars[i] + "=" + _method).c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + "CGI/1.1").c_str()); i++;
	// _env[i] = strdup((envVars[i] + "=" + _method).c_str()); i++;
	string url = request.getUrl();
	int scriptNameStart = url.find("/cgi/") + 5;
	int scriptNameEnd = url.find("/", scriptNameStart) - 1;
	int queryStart = url.find("?");
	string scriptName = url.substr(scriptNameStart, scriptNameEnd);
	string pathInfo = url.substr(scriptNameEnd + 2, queryStart);
	string queryString = (queryStart != (int)string::npos )? url.substr(queryStart, string::npos) : "";
	_env[i] = strdup((envVars[i] + "=" + pathInfo).c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + "./" + pathInfo).c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + "./" + scriptName).c_str()); i++;
	_env[i] = strdup((envVars[i] + "=" + queryString).c_str()); i++;
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
		_env[i++] = nullptr;
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
		chdir("cgi-bin/site");
		if(execlp("/usr/bin/php", "php", file.c_str(), NULL) == -1)
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
