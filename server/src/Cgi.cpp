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
	string url = request.getRoutedTarget();
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
		// _pathTranslated = "." + _docRoot + _pathInfo.substr(1);
		_pathTranslated = _pathInfo;
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
	int fileFd = open(this->_pathTranslated.c_str(), O_RDONLY);
	if (fileFd == -1)
	{
		LOG_RED("cgi resource not found");
		throw CgiExceptionNotFound();
	}
	dup2(fileFd, STDIN_FILENO);
	close(fileFd);
}

void Cgi::passAsOutput(void)
{
	int fileFd = open(this->_pathTranslated.c_str(), O_WRONLY);
	if (fileFd == -1)
	{
		LOG_RED("cgi resource not found");
		throw CgiExceptionNotFound();
	}
	dup2(fileFd, STDOUT_FILENO);
	close(fileFd);
}

static void free_env(char **env)
{
	for (int i = 0; env[i]; i++)
		free(env[i]);
	delete[] env;
}

void Cgi::init_cgi(int client_fd, int cgi_out)
{
	std::string file;
	std::string executable;
	char **args;
	int pipefd[2];

	args = NULL;
	(void)client_fd;
	int stdout_init = dup(STDOUT_FILENO);
	int stdin_init = dup(STDIN_FILENO);
	executable = _scriptName;
	pipe(pipefd);
	#ifdef SHOW_LOG_CGI
		LOG_GREEN("Init cgi executable: " << executable);
	#endif
	if (!_selfExecuting && this->_method == "GET")
	{
		#ifdef SHOW_LOG_CGI
			cout << "GET" << endl;
		#endif
		passAsInput();
		close(pipefd[1]);
	}
	if (this->_method == "POST")
	{
		rewind(this->_infile);
		int infileFd = fileno(this->_infile);
		lseek(infileFd, 0, SEEK_SET);
		if (dup2(infileFd, STDIN_FILENO == -1))
		{
			std::cout << "dup2 failed" << std::endl;
			throw std::exception();
		}
	}
	file = _pathInfo;
	int pid_cgi;
	int pid_timer = fork();
	if (pid_timer == 0)
	{
		sleep(2);
		exit(0);
	}
	else if (pid_timer == -1)
		throw std::runtime_error("error forking");
	else
	{
		pid_cgi = fork();
		if (pid_cgi == -1)
		{
			close(cgi_out);
			close(pipefd[0]);
		}
		if (pid_cgi == 0)
		{
			dup2(cgi_out, STDOUT_FILENO);
			close(cgi_out);
			char **env = mapToStringArray(this->_env);
			if (execve(executable.c_str(), args, env) == -1)
			{
				#ifdef SHOW_LOG_CGI
				std::cerr << "error executing cgi" << std::endl;
				#endif
			}
			free_env(env);
			exit(1); // throw internal server error if this occurs
		}
	}
	cgi_exit(pid_timer, pid_cgi);
	dup2(stdin_init, STDIN_FILENO);
	dup2(stdout_init, STDOUT_FILENO);
}

void Cgi::cgi_exit(int pid_timer, int pid_cgi)
{
	int wstatus;
	while (waitpid(pid_timer, NULL, WNOHANG) == 0)
	{
		if (waitpid(pid_cgi, &wstatus, WNOHANG) == pid_cgi)
			break;
	}
	if (!WIFEXITED(wstatus))
	{
		kill(pid_cgi, SIGKILL);
		throw std::runtime_error("error executing cgi");
	}
}

const char *Cgi::CgiExceptionNotFound::what() const throw()
{
	return "400";
}