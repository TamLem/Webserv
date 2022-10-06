#include "Cgi/Cgi.hpp"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>

//print string map
void printMap(std::map<std::string, std::string> map)
{
	std::map<std::string, std::string>::iterator it = map.begin();
	while (it != map.end())
	{
		std::cout << it->first << " : " << it->second << std::endl;
		it++;
	}
}

Cgi::Cgi(Request &request, ConfigStruct configStruct, FILE *infile):
	_selfExecuting(false), _confStruct(configStruct), _infile(infile)
{
	#ifdef SHOW_CONSTRUCTION
		std::cout << GREEN << "Cgi Constructor called for " << this << RESET << std::endl;
	#endif

	_docRoot = configStruct.root;
	_method = request.getMethod();
	_chunked = false;
	if(request.getHeaderFields().find("transfer-encoding")->second == "chunked")
		_chunked = true;
	// string url = request.getDecodedTarget(); //returning empty string fix in request
	string url = request.getUrl(); // AE @tam do you need the decoded or routed target?
	// string url = request.getRoutedTarget();
	string extension = url.substr(url.find_last_of("."));

	if (url.find("/cgi/") != string::npos)
		_selfExecuting = true;

	int queryStart = url.find("?");
	if (_selfExecuting)
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
	_queryString = request.getQuery().length() > 1 ? request.getQuery().substr(1) : "";
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
	//print header fields
	_env["REQUEST_METHOD"] = _method;
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["PATH_INFO"] = _pathInfo;
	_env["PATH_TRANSLATED"] = _pathTranslated;
	_env["SCRIPT_NAME"] = "";
	_env["QUERY_STRING"] = _queryString;
	_env["SERVER_NAME"] = reqHeaders.find("host")->second.substr(0, reqHeaders.find("host")->second.find(":"));
	_env["SERVER_PORT"] =  reqHeaders.find("host")->second.substr(reqHeaders.find("host")->second.find(":") + 1);
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	if (_method == "POST")
	{
		_env["CONTENT_TYPE"] = reqHeaders["content-type"];
		_env["CONTENT_LENGTH"] = reqHeaders["content-length"];
		_env["REMOTE_HOST"] = reqHeaders["host"];
		_env["REMOTE_ADDR"] = reqHeaders["X-Forwarded-For"];
	}
	_env["REDIRECT_STATUS"] = "200";
	std::map<std::string, std::string>::const_iterator it = reqHeaders.begin();
	while (it != reqHeaders.end()) {
		_env["HTTP_" + it->first] = it->second;
		it++;
	}

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

void Cgi::passAsInput(void)
{
	int fileFd = open("./42tester/YoupiBanane/youpi.bla", O_RDONLY);
	if (fileFd == -1)
	{
		cout << "path not found" << endl;
		return;
	}
	dup2(fileFd, STDIN_FILENO);
	close(fileFd);
}

void Cgi::passAsOutput(void)
{
	int fileFd = open("./42tester/YoupiBanane/youpi.bla", O_WRONLY);
	if (fileFd == -1)
	{
		cout << "path not found" << endl;
		return;
	}
	dup2(fileFd, STDOUT_FILENO);
	close(fileFd);
}

static size_t _handleChunked(int clientFd)
{
	// read the line until /r/n is read
	char buffer[2];
	std::string hex = "";
	while (hex.find("\r\n") == std::string::npos)
	{
		int returnvalue = read(clientFd, buffer, 1);
		buffer[1] = '\0';
		if (returnvalue < 1)
			return -1;
		hex.append(buffer);
	}
	hex.resize(hex.size() - 2);

	std::stringstream hexadecimal;
	hexadecimal << hex;

	size_t length = 0;
	// int length2 = 0;
	// length << std::hex << hexadecimal.str();
	hexadecimal >> std::hex >> length;

	return (length);
}

void Cgi::init_cgi(int client_fd, int cgi_out)
{
	std::string file;
	std::string executable;
	char **args;
	int pipefd[2];

	args = NULL;
	int stdout_init = dup(STDOUT_FILENO);
	int stdin_init = dup(STDIN_FILENO);
	executable = _scriptName;
	if (this->_chunked)
		_handleChunked(client_fd);
	pipe(pipefd);
	#ifdef SHOW_LOG_CGI
		std::cout << GREEN << "Init cgi..." << executable << endl;
	#endif
	if (!_selfExecuting && this->_method == "GET")
	{
		cout << "GET" << endl;
		passAsInput();
		close(pipefd[1]);
	}
	if (this->_method == "POST")
	{
		long	lSize = ftell(this->_infile);
		#ifdef SHOW_LOG_CGI
			cout << "lSize: " << lSize << endl;
		#endif
		rewind(this->_infile);
		int infileFd = fileno(this->_infile);
		lseek(infileFd, 0, SEEK_SET);
		if (dup2(infileFd, STDIN_FILENO == -1))
		{
			std::cout << "dup2 failed" << std::endl;
			return ; //add 500
		}
		// passAsOutput();
	}
	file = _pathInfo;
	int pid = fork();
	if (pid == -1)
	{
		close(cgi_out);
		close(pipefd[0]);
	}
	if (pid == 0)
	{
		dup2(cgi_out, STDOUT_FILENO);
		close(cgi_out);
		if (execve(executable.c_str(), args, mapToStringArray(_env)) == -1)
		{
			std::cerr << "error executing cgi" << std::endl;
		}
		exit(1); // throw internal server error if this occurs
	}
	wait(NULL);
	dup2(stdin_init, STDIN_FILENO);
	dup2(stdout_init, STDOUT_FILENO);
}

// void Cgi::cgi_response(int fd)
// {
// 	std::string file;
// 	std::string executable;
// 	char **args;
// 	int pipefd[2];

// 	args = NULL;
// 	executable = _scriptName;
// 	if (this->_chunked)
// 		_handleChunked(fd);
// 	pipe(pipefd);
// 	int stdout_init = dup(STDOUT_FILENO);
// 	int stdin_init = dup(STDIN_FILENO);
// 	std::cout << GREEN << "Executing CGI..." << std::endl;
// 	cout << "executable: " << executable << endl;
// 	if (!_selfExecuting && this->_method == "GET")
// 	{
// 		cout << "GET" << endl;
// 		passAsInput();
// 		dup2(pipefd[1], STDOUT_FILENO);
// 		close(pipefd[1]);
// 	}
// 	if (this->_method == "POST")
// 	{
// 		dup2(fd, STDIN_FILENO);
// 		passAsOutput();
// 	}
// 	file = _pathInfo;
// 	int pid = fork();
// 	if (pid == 0)
// 	{
// 		if (execve(executable.c_str(), args, mapToStringArray(_env)) == -1)
// 		{
// 			std::cerr << "error executing cgi" << std::endl;
// 		}
// 		exit(0);
// 	}
// 	wait(NULL);
// 	dup2(stdin_init, STDIN_FILENO);
// 	dup2(stdout_init, STDOUT_FILENO);
// 	cout << "cgi response done" << endl;
// 	std::string header = "HTTP/1.1 200 OK\r\n"; //cgi status code
// 	send(fd, header.c_str(), header.size(), 0);
// 	if (this->_method == "POST")
// 	{
// 		close(pipefd[0]);
// 		close(fd);
// 		return ;
// 	}
// 	char buf[1024];
// 	buf[1023] = '\0';
// 	int n;
// 	while((n = read(pipefd[0], buf, 1023)) > 0)
// 	{
// 		#ifdef SHOW_LOG
// 			cout << "cgi output: " << buf << endl;
// 		#endif
// 		send(fd, buf, n, 0); // make sure to not send any data to the client here!!! you need to put the data into the responseMap[clientFd] and then add a write event !!!!! @Tam
// 	}
// 	close(pipefd[0]);
// 	close(fd);
// }