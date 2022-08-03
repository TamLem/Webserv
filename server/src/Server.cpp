#include "Server.hpp"
#include "Config.hpp"
#include "Cgi/Cgi.hpp"
#include <sys/stat.h> // stat

// Server::Server(void)
// {
// 	std::cout << "Server default constructor called for " << this << std::endl;
// 	handle_signals();
// }

Server::Server(Config* config): _config(config), _socketHandler(new SocketHandler(_config))
{
	#ifdef SHOW_LOG
		std::cout << GREEN << "Server constructor called for " << this << RESET << std::endl;
	#endif
	handle_signals();
}

Server::~Server(void)
{
	delete _socketHandler;
	#ifdef SHOW_LOG
		std::cout << RED << "server deconstructor called for " << this << RESET << std::endl;
	#endif
}


void Server::handle_signal(int sig)
{
	if (sig == SIGINT)
	{
		std::cerr << BLUE << "SIGINT detected, terminating server now" << RESET << std::endl;
		keep_running = 0;
	}
	else if (sig == SIGPIPE)
	{
		std::cerr << RED << "SIGPIPE detected, will end now" << RESET << std::endl;
		keep_running = 0;
	}
}

// check if signal is forbidden!!!!!!!!!!!!!!!!!
void	Server::handle_signals(void)
{
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	// signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, handle_signal);
	// signal(SIGPIPE, handle_signal);
}

void Server::runEventLoop()
{
	while(keep_running)
	{
		std::cout << "server running " << std::endl;
		int numEvents = this->_socketHandler->getEvents();
		if (numEvents == 0)
		{
			this->_socketHandler->removeInactiveClients();	// remove inactive clients
		}
		for (int i = 0; i < this->_socketHandler->getNumEvents() ; ++i)
		{
			std::cout << "no. events: " << this->_socketHandler->getNumEvents() << " ev:" << i << std::endl;
			this->_socketHandler->acceptConnection(i);
			if (this->_socketHandler->readFromClient(i) == true)
			{
				// this->_readRequestHead(this->_socketHandler->getFD()); // read 1024 charackters or if less until /r/n/r/n is found
				handleRequest(/*this->_requestHead, */this->_socketHandler->getFD());
				// continue;
			}
			this->_socketHandler->removeClient(i);
		}
	}
}

static bool staticFileExists(const std::string& target)
{
	struct stat statStruct;

	if (stat(target.c_str(), &statStruct) == 0)
			return (true);
	return (false);
}

void Server::handleGET(const Request& request)
{
	_response.setProtocol(PROTOCOL);
	if (request.isFile == false && staticFileExists(request.getUri()) == false)
	{
		if ((this->_currentLocationKey.empty() == false
				&& (this->_currentConfig.location.find(_currentLocationKey)->second.autoIndex == true))
				|| this->_currentConfig.autoIndex == true)
			_response.createIndex(request.getUri());
		else
			_response.createBody(request.getUri());
	}
	else
		_response.createBody(request.getUri());
	_response.addHeaderField("Server", this->_currentConfig.serverName);
	_response.addDefaultHeaderFields();
	_response.setStatus("200");
}

void Server::handlePOST(const Request& request)
{
	std::ofstream outFile;
	outFile.open("./uploads/" + request.getBody());
	if (outFile.is_open() == false)
		throw std::exception();
	outFile << request.getBody() << "'s content. Server: " << this->_config->getConfigStruct("weebserv").serverName;
	outFile.close();

	_response.setProtocol(PROTOCOL);
	_response.createBody("./pages/post_test.html");
	_response.addHeaderField("Server", this->_currentConfig.serverName);
	_response.addDefaultHeaderFields();
	_response.setStatus("200");
}

void Server::handleERROR(const std::string& status)
{
	_response.setStatus(status);
	_response.setProtocol(PROTOCOL);
	_response.createErrorBody();
	_response.addHeaderField("Server", this->_currentConfig.serverName);
	_response.addDefaultHeaderFields();
}

void Server::applyCurrentConfig(const Request& request)
{
	std::map<std::string, std::string> headerFields;
	headerFields = request.getHeaderFields();
	std::string host = headerFields["host"];
	this->_currentConfig = this->_config->getConfigStruct(host);
}

int Server::routeFile(Request& request, std::map<std::string, LocationStruct>::const_iterator it, const std::string& uri)
{
	std::string extension;
	size_t ext_len;
	size_t uri_len;
	std::string result;
	
	uri_len = uri.length();
	ext_len = it->first.length() - 1;
	extension = it->first.substr(1, ext_len);
	if (uri_len > ext_len && uri.compare(uri_len - ext_len, ext_len, extension) == 0)
	{
		if (it->second.root.empty())
			result = this->_currentConfig.root;
		else
			result = it->second.root;
		result += uri.substr(uri.find_last_of('/') + 1);
		request.setUri(result);
		_currentLocationKey = it->first;
		#ifdef SHOW_LOG
			std::cout  << YELLOW << "FILE ROUTING RESULT!: " << request.getUri() << " for location: " << _currentLocationKey  << std::endl;
		#endif
		return (0);
	}
	return (1);
}

void Server::routeDir(Request& request, std::map<std::string, LocationStruct>::const_iterator it, const std::string& uri, int& max_count)
{
	std::string path;
	std::string result;
	
	path = it->first;
	// if (path != "/")
	// 	path = "/" + path;
	#ifdef SHOW_LOG_2
		std::cout  << BLUE << "path: " << path << std::endl;
	#endif
	int i = 0;
	int segments = 0;
	if (uri.length() >= path.length()) //path has to be checked until the end and segments need to be counted
	{
		while (path[i] != '\0')
		{
			if (path[i] != uri[i])
			{
				segments = 0;
				break ;
			}
			if (path[i] == '/')
				segments++;
			i++;
		}
		if (uri[i - 1] != '\0' && uri[i - 1] != '/') // carefull with len = 0!
			segments = 0;
	}
	if (segments > max_count)
	{
		max_count = segments;
		if (it->second.root.empty())
			result = this->_currentConfig.root;
		else
			result = it->second.root;
		result += uri.substr(i);
		if (*result.rbegin() == '/')
		{
			request.isFile = false;
			if (it->second.indexPage.empty() == false)
				result += it->second.indexPage;
			else
				result += this->_currentConfig.indexPage;
			// else
			// 	std::cerr << BOLD << RED << "ERROR: autoindex not implemented!" << RESET << std::endl;
		}
		request.setUri(result);
		_currentLocationKey = it->first;
		#ifdef SHOW_LOG_2
			std::cout  << YELLOW << "DIR MATCH!: " << request.getUri() << " for location: " << _currentLocationKey << std::endl;
		#endif
	}
}

void Server::routeDefault(Request& request)
{
	std::string result;

	result = this->_currentConfig.root + request.getUri().substr(1);
	if (*result.rbegin() == '/')
	{
		request.isFile = false;
		// if (this->_currentConfig.autoIndex == false)
			result += this->_currentConfig.indexPage;
		// else
		// 	std::cerr << BOLD << RED << "ERROR: autoindex not implemented!" << RESET << std::endl;
	}
	request.setUri(result);
	_currentLocationKey = "";
	#ifdef SHOW_LOG
		std::cout  << YELLOW << "DEFAULT ";
	#endif
}

void Server::matchLocation(Request& request)
{
	int max_count = 0;
	std::string uri = request.getUri();
	#ifdef SHOW_LOG_2
	std::cout  << RED << "uri: " << uri << std::endl;
	for (std::map<std::string, LocationStruct>::const_iterator it = this->_currentConfig.location.begin(); it != this->_currentConfig.location.end(); ++it)
	{
		std::cout << RED << it->first << ": "
		<< it->second.root << " is dir: " << it->second.isDir << RESET << "\n";
	}
	#endif
	for (std::map<std::string, LocationStruct>::const_iterator it = this->_currentConfig.location.begin(); it != this->_currentConfig.location.end(); ++it)
	{
		if (it->second.isDir == false)
		{
			if (routeFile(request, it, uri) == 0)
				return ;
		}
		if (it->second.isDir == true)
			routeDir(request, it, uri, max_count);
	}
	if (max_count == 0)
		routeDefault(request);
	#ifdef SHOW_LOG
		std::cout  << YELLOW << "DIR ROUTING RESULT!: " << request.getUri() << " for location: " << _currentLocationKey  << std::endl;
	#endif
}

std::string Server::percentDecoding(const std::string& str)
{
	std::stringstream tmp;
	// std::string str = request.getUri();
	char c;
	int i = 0;
	while (str[i] != '\0')
	{
		if (str[i] == '%')
		{
			// valid ascii check
			if (str[i + 1] == '\0' || str[i + 2] == '\0')
				throw InvalidHex();
			c = char(strtol(str.substr(i + 1, 2).c_str(), NULL, 16));
			if (c == 0)
				throw InvalidHex();
			tmp << c;
			// std::cerr << RED << str.substr(i + 1, 2) << RESET << std::endl;
			i += 3;
		}
		else
		{
			tmp << str[i];
			i++;
		}
	}
	// request.setUri(tmp.str());
	return(tmp.str());
}

void Server::checkLocationMethod(const Request& request) const
{
	if (this->_currentLocationKey.empty() == true)
		return ;
	if (this->_currentConfig.location.find(_currentLocationKey)->second.allowedMethods.count(request.getMethod()) != 1)
		throw MethodNotAllowed();
}

void Server::handleRequest(/*const std::string& buffer, */int fd) // maybe breaks here
{
	this->_response.clear();
	try
	{
		this->_readRequestHead(fd); // read 1024 charackters or if less until /r/n/r/n is found
		Request request(this->_requestHead);
		this->applyCurrentConfig(request);
		//normalize uri (in Request)
		//compression (merge slashes)
		//resolve relative paths
		//determine location
		this->matchLocation(request); // AE location with ü (first decode only unreserved chars?)
		request.setUri(this->percentDecoding(request.getUri()));
		request.setQuery(this->percentDecoding(request.getQuery()));
		#ifdef SHOW_LOG
			std::cout  << YELLOW << "URI after percent-decoding: " << request.getUri() << std::endl;
		#endif
		request.setUri("." + request.getUri());
		//check method
		checkLocationMethod(request);
		if (this->_requestHead.find("/cgi/") != std::string::npos)
			cgi_handle(request, fd);
		else if (request.getMethod() == "POST")
			handlePOST(request);
		else
		{
			handleGET(request);
			// lseek(fd, 0, SEEK_END); // sets the filedescriptor to EOF so that, check this again!!!!!!!!!
		}
	}
	catch (std::exception& exception)
	{
		std::string code = exception.what();
		if (_response.getMessageMap().count(code) != 1)
			code = "500";
		handleERROR(code);
	}
	// std::cerr << BLUE << "Remember and fix: Tam may not send response inside of cgi!!!" << RESET << std::endl;
	this->_response.sendResponse(fd); // AE Tam may not send response inside of cgi
}

bool Server::_crlftwoFound()
{
	if (this->_requestHead.find(CRLFTWO) != std::string::npos)
		return (true);
	else
		return (false);
}

bool Server::_isPrintableAscii(char c)
{
	if (c > 126 || c < 0)
		return (false);
	else
		return (true);
}

// read 1024 charackters or if less until /r/n/r/n is found
void Server::_readRequestHead(int fd)
{
	this->_requestHead.clear();
	size_t charsRead = 0;
	bool firstLineBreak = false;
	int n = 0;
	char buffer[2];
	while (charsRead < MAX_REQUEST_HEADER_SIZE)
	{
		n = read(fd, buffer, 1);
		if (n < 0) // read had an error reading from fd
		{
			std::cerr << RED << "READING FROM FD " << fd << " FAILED" << std::endl;
			perror(NULL); // check if forbidden!!!!!!!!
			std::cerr << RESET << std::endl;
			throw Server::InternatServerErrorException();
		}
		else if (n == 0) // read reached eof
			break ;
		else // append one character from client request if it is an ascii-char
		{
			buffer[1] = '\0';
			if (!this->_isPrintableAscii(buffer[0]))
			{
				std::cout << RED << "NON-ASCII CHAR FOUND IN REQUEST" << RESET << std::endl;
				throw Server::BadRequestException();
			}
			this->_requestHead.append(buffer);
			++charsRead;
			if (this->_crlftwoFound() == true)
				break ;
		}
		if (firstLineBreak == false && this->_requestHead.find(CRLF) != std::string::npos)
		{
			firstLineBreak = true;
		}
		if (firstLineBreak == false && charsRead >= MAX_REQUEST_LINE_SIZE)
		{
			std::cout << RED << "FIRST LINE TOO LONG" << RESET << std::endl;
			throw Server::FirstLineTooLongException();
		}
	}
	if (charsRead <= MAX_REQUEST_HEADER_SIZE && this->_crlftwoFound() == true)
	{
		#ifdef SHOW_LOG
			std::cout << YELLOW << "Received->" << RESET << this->_requestHead << YELLOW << "<-Received on fd: " << fd << RESET << std::endl;
		#endif
	}
	else /*if (charsRead >= MAX_REQUEST_HEADER_SIZE && this->_crlftwoFound() == false)*/
	{
		std::cout << RED << "HEAD BIGGER THAN " << MAX_REQUEST_HEADER_SIZE << " OR NO CRLFTWO FOUND (incomplete request)" << RESET << std::endl;
		throw Server::BadRequestException();
	}
}

// Exceptions
const char* Server::InternatServerErrorException::what(void) const throw()
{
	return ("500");
}

const char* Server::BadRequestException::what(void) const throw()
{
	return ("400");
}

const char* Server::FirstLineTooLongException::what(void) const throw()
{
	return ("414");
}

void cgi_handle(Request& request, int fd)
{
	Cgi newCgi(request);

	newCgi.printEnv();
	int cgiPipe[2];
	pipe(cgiPipe);
	//initiate cgi response
	//listen to event on cgiPipe[0]
	//if event is read, read from cgiPipe[0] and write to fd
	//if event is write, write to cgiPipe[1]
	//if event is error, close cgiPipe[0] and cgiPipe[1]
	//if event is hangup, close cgiPipe[0] and cgiPipe[1]
	//if event is timeout, close cgiPipe[0] and cgiPipe[1]
	//if event is terminate, close cgiPipe[0] and cgiPipe[1]
	//if event is close, close cgiPipe[0] and cgiPipe[1]
	
	
	newCgi.cgi_response(fd);
}

const char* Server::InvalidHex::what() const throw()
{
	return ("400");
}

const char* Server::MethodNotAllowed::what() const throw()
{
	return ("405");
}
