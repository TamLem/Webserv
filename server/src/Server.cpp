#include "Server.hpp"
#include "Config.hpp"
#include "Cgi/Cgi.hpp"
#include <dirent.h> // dirent, opendir
#include <cstdio> // remove


static std::string staticReplaceInString(std::string str, std::string tofind, std::string toreplace)
{
		size_t position = 0;
		for ( position = str.find(tofind); position != std::string::npos; position = str.find(tofind,position) )
		{
				str.replace(position , tofind.length(), toreplace);
		}
		return(str);
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

void Server::handle_signals(void)
{
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	// signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, handle_signal);
	// signal(SIGPIPE, handle_signal);
}

Server::Server(Config* config): _config(config), _socketHandler(new SocketHandler(_config))
{
	#ifdef SHOW_CONSTRUCTION
		std::cout << GREEN << "Server constructor called for " << this << RESET << std::endl;
	#endif
	#ifdef __APPLE__
		handle_signals();
	#endif
}

Server::~Server(void)
{
	delete _socketHandler;
	#ifdef SHOW_CONSTRUCTION
		std::cout << RED << "server deconstructor called for " << this << RESET << std::endl;
	#endif
}

void Server::runEventLoop()
{
	while(keep_running)
	{

		#ifdef SHOW_LOG_2
		std::cout << "server running " << std::endl;
		#endif
		int numEvents = this->_socketHandler->getEvents();
		if (numEvents == 0)
		{
			this->_socketHandler->removeInactiveClients();	// remove inactive clients
			this->_response.clearResponseMap();
		}
		for (int i = 0; i < numEvents; ++i)
		{
			#ifdef SHOW_LOG_2
			std::cout << "no. events: " << this->_socketHandler->getNumEvents() << " ev:" << i << std::endl;
			#endif
			this->_socketHandler->acceptConnection(i);
			if (this->_socketHandler->removeClient(i) == true)
				this->_response.removeFromResponseMap(this->_socketHandler->getFD(i));
			else if (this->_socketHandler->readFromClient(i) == true)
			{
				std::cout << BLUE << "read from client" << this->_socketHandler->getFD(i) << RESET << std::endl;
				handleRequest(this->_socketHandler->getFD(i));
				this->_socketHandler->setWriteable(i);
			}
			else if (this->_socketHandler->writeToClient(i) == true)
			{
				std::cout << BLUE << "write to client" << this->_socketHandler->getFD(i) << RESET << std::endl;
				//this->responseMap.count(i).respond()
				//if (this->responseMap.count(i).isDone())
					//close(fd)
					//delete the (fd, pair)reponse
				// this->_handleResponse(i);
				if (this->_response.sendResponse(this->_socketHandler->getFD(i)) == true)
				{
					if (this->_socketHandler->removeClient(i, true) == true)
						this->_response.removeFromResponseMap(this->_socketHandler->getFD(i));
				}
			}
		}
	}
}

void Server::handleGET(const Request& request)
{
	_response.setProtocol(PROTOCOL);
	if (request.isFile == false && targetExists(request.getRoutedTarget() + request.indexPage) == false)
	{
		if ((this->_currentLocationKey.empty() == false
		&& (this->_currentConfig.location.find(_currentLocationKey)->second.autoIndex == true))
		|| this->_currentConfig.autoIndex == true)
		{
			_response.createIndex(request);
		}
		else
			_response.createBodyFromFile(request.getRoutedTarget() + request.indexPage);
	}
	else
		_response.createBodyFromFile(request.getRoutedTarget() + request.indexPage);
	_response.addHeaderField("Server", this->_currentConfig.serverName);
	_response.addDefaultHeaderFields();
	_response.setStatus("200");
}

void Server::handlePOST(const Request& request)
{
	std::ofstream outFile;
	// outFile.open(UPLOAD_DIR + request.getBody()); // AE body is not read anymore and therefore empty
	std::string teststring = "myfile=Disaster-Girl.jpg"; // AE remove this
	std::string file = staticReplaceInString(teststring, "myfile=", ""); // AE this has to be request.getBody() instead of teststring
	std::string target = UPLOAD_DIR + file;
	outFile.open(target.c_str()); // AE body is not read anymore and therefore empty
	if (outFile.is_open() == false)
		throw std::exception();
	outFile << request.getBody();
	outFile.close();

	_response.setProtocol(PROTOCOL);
	_response.createBodyFromFile("./server/data/pages/post_test.html");
	_response.addHeaderField("Server", this->_currentConfig.serverName);
	_response.addDefaultHeaderFields();
	_response.setStatus("201");
}

static void staticRemoveTarget(const std::string& path)
{
	if (targetExists(path) == false)
		throw Response::ERROR_404();
	if (remove(path.c_str()) != 0)
	{
		if (errno == EACCES)
			throw Response::ERROR_403();
		if (errno == ENOTEMPTY)
			throw Request::DirectoryNotEmpty();
		else
			throw Response::ERROR_500();
	}
}

void Server::handleDELETE(const Request& request)
{
	staticRemoveTarget(request.getRoutedTarget());
	_response.setProtocol(PROTOCOL);
	_response.setBody("");
	_response.addHeaderField("Server", this->_currentConfig.serverName);
	_response.setStatus("200");
}

void Server::handleERROR(const std::string& status)
{
	_response.setStatus(status);
	_response.setProtocol(PROTOCOL);
	if (this->_currentConfig.errorPage.count(status) == 1 && this->loopDetected == false)
	{
		this->loopDetected = true;
		_response.createBodyFromFile("." + this->_currentConfig.errorPage[status]);
	}
	else
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

// static std::string staticRemoveTrailingSlash(std::string string)
// {
// 	if (*string.rbegin() == '/')
// 		string = string.substr(0, string.length() - 1);
// 	return (string);
// }

// static bool staticTargetIsFile(const std::string& target)
// {
// 	struct stat statStruct;

// 	// if (stat(target.c_str(), &statStruct) != 0)
// 	// 	throw InternalServerError();
// 	// if (statStruct.st_mode & S_IFREG == 0 && statStruct.st_mode & S_IFDIR == 0)
// 	// 	throw InternalServerError();
// 	// else if (statStruct.st_mode & S_IFREG)
// 	// 	return (true);
// 	// else
// 	// 	return (false);

// 	// error will return false
// 	if (stat(target.c_str(), &statStruct) == 0)
// 	{
// 		if (statStruct.st_mode & S_IFREG)
// 			return (true);
// 	}
// 	return (false);
// }

// static bool staticTargetIsDir(const std::string& target)
// {
// 	struct stat statStruct;

// 	// error will return false
// 	if (stat(target.c_str(), &statStruct) == 0)
// 	{
// 		if (statStruct.st_mode & S_IFDIR)
// 			return (true);
// 	}
// 	return (false);
// }

//https://stackoverflow.com/questions/29310166/check-if-a-fstream-is-either-a-file-or-directory
// static bool staticIsFile(const std::string& fileName)
// {
// 	std::ifstream fileOrDir(fileName);
// 	//This will set the fail bit if fileName is a directory (or do nothing if it is already set
// 	fileOrDir.seekg(0, std::ios::end);
// 	if( !fileOrDir.good())
// 		return (false);
// 	return (true);
// }

int Server::routeFile(Request& request, std::map<std::string, LocationStruct>::const_iterator it, const std::string& target)
{
	std::string extension;
	size_t ext_len;
	size_t target_len;
	std::string result;

	target_len = target.length();
	ext_len = it->first.length() - 1;
	extension = it->first.substr(1, ext_len);
	if (target_len > ext_len && target.compare(target_len - ext_len, ext_len, extension) == 0)
	{
		if (it->second.root.empty())
			result = this->_currentConfig.root;
		else
			result = it->second.root;
		result += target.substr(target.find_last_of('/') + 1);
		request.setRoutedTarget("." + result);
		_currentLocationKey = it->first;
		#ifdef SHOW_LOG
			std::cout  << YELLOW << "FILE ROUTING RESULT!: " << request.getRoutedTarget() << " for location: " << _currentLocationKey  << std::endl;
		#endif
		return (0);
	}
	return (1);
}

void Server::routeDir(Request& request, std::map<std::string, LocationStruct>::const_iterator it, const std::string& target, int& max_count)
{
	std::string path;
	std::string result;

	path = it->first;
	#ifdef SHOW_LOG_2
		std::cout  << BLUE << "path: " << path << std::endl;
	#endif
	int i = 0;
	int segments = 0;
	if (target.length() >= path.length())
	{
		while (path[i] != '\0')
		{
			if (path[i] != target[i])
			{
				segments = 0;
				break ;
			}
			if (path[i] == '/')
				segments++;
			i++;
		}
		if (target[i - 1] != '\0' && target[i - 1] != '/')
			segments = 0;
	}
	if (segments > max_count)
	{
		max_count = segments;
		if (it->second.root.empty())
			result = this->_currentConfig.root;
		else
			result = it->second.root;
		result += target.substr(i);
		if (*result.rbegin() == '/')
		{
			request.isFile = false;
			if (it->second.indexPage.empty() == false)
				request.indexPage = it->second.indexPage;
			else
				request.indexPage = this->_currentConfig.indexPage;
		}
		request.setRoutedTarget("." + result);
		_currentLocationKey = it->first;
		#ifdef SHOW_LOG_2
			std::cout  << YELLOW << "DIR MATCH!: " << request.getRoutedTarget() << " for location: " << _currentLocationKey << std::endl;
		#endif
	}
}

void Server::routeDefault(Request& request)
{
	std::string result;

	result = this->_currentConfig.root + request.getDecodedTarget().substr(1);
	if (*result.rbegin() == '/')
	{
		request.isFile = false;
		request.indexPage = this->_currentConfig.indexPage;
	}
	request.setRoutedTarget("." + result);
	_currentLocationKey = "";
	#ifdef SHOW_LOG
		std::cout  << YELLOW << "DEFAULT ";
	#endif
}

void Server::matchLocation(Request& request)
{
	int max_count = 0;
	std::string target = request.getDecodedTarget();
	#ifdef SHOW_LOG_2
	std::cout  << RED << "target: " << target << std::endl;
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
			if (routeFile(request, it, target) == 0)
				return ;
		}
		if (it->second.isDir == true)
			routeDir(request, it, target, max_count);
	}
	if (max_count == 0)
		routeDefault(request);
	#ifdef SHOW_LOG
		std::cout  << YELLOW << "DIR ROUTING RESULT!: " << request.getRoutedTarget() << " for location: " << _currentLocationKey  << std::endl;
	#endif
}

static std::string staticPercentDecodingFix(std::string target)
{
	std::string accent;
	accent += (const char)204;
	accent += (const char)136;

	std::string ü;
	ü += (const char)195;
	ü += (const char)188;

	std::string ä;
	ä += (const char)195;
	ä += (const char)164;

	std::string ö;
	ö += (const char)195;
	ö += (const char)182;

	target = staticReplaceInString(target, "u" + accent, ü);
	target = staticReplaceInString(target, "a" + accent, ä);
	target = staticReplaceInString(target, "o" + accent, ö);
	return (target);
}

std::string Server::percentDecoding(const std::string& str)
{
	std::stringstream tmp;
	char c;
	int i = 0;
	while (str[i] != '\0')
	{
		if (str[i] == '%')
		{
			if (str[i + 1] == '\0' || str[i + 2] == '\0')
				throw InvalidHex();
			c = char(strtol(str.substr(i + 1, 2).c_str(), NULL, 16));
			if (c == 0)
				throw InvalidHex();
			tmp << c;
			i += 3;
		}
		else
		{
			tmp << str[i];
			i++;
		}
	}
	return (staticPercentDecodingFix(tmp.str()));
}

void Server::checkLocationMethod(const Request& request) const
{
	if (this->_currentLocationKey.empty() == true)
		return ;
	if (this->_currentConfig.location.find(_currentLocationKey)->second.allowedMethods.count(request.getMethod()) != 1)
		throw MethodNotAllowed();
}

void Server::handleRequest(int fd)
{
	bool isCgi = false;
	this->_response.clear();
	this->loopDetected = false;
	try
	{
		this->_readRequestHead(fd); // read 1024 charackters or if less until /r/n/r/n is found
		Request request(this->_requestHead);
		this->applyCurrentConfig(request);
		//normalize target (in Request)
		//compression (merge slashes)
		//resolve relative paths
		//determine location
		request.setDecodedTarget(this->percentDecoding(request.getRawTarget()));
		request.setQuery(this->percentDecoding(request.getQuery()));
		if (this->_requestHead.find("/cgi/") != std::string::npos || this->_requestHead.find(".bla") != std::string::npos) // needs to be changed so it accepts the cgi-bin instead andd also the file extensions
			isCgi = true; // pass the relevant ConfigStruct to CGI
		#ifdef SHOW_LOG
			std::cout  << YELLOW << "URI after percent-decoding: " << request.getDecodedTarget() << std::endl;
		#endif
		this->matchLocation(request); // AE location with ü (first decode only unreserved chars?)
		// request.setRoutedTarget("." + request.getRoutedTarget());
		//check method
		checkLocationMethod(request);
		if (isCgi == true)
			cgi_handle(request, fd);
		else if (request.getMethod() == "POST" || request.getMethod() == "PUT")
			handlePOST(request);
		else if (request.getMethod() == "DELETE")
			handleDELETE(request);
		else
		{
			handleGET(request);
		}
	}
	catch (std::exception& exception)
	{
		std::string code = exception.what();
		if (_response.getMessageMap().count(code) != 1)
			code = "500";
		try
		{
		handleERROR(code);
		}
		catch(const std::exception& exception)
		{
			std::string code = exception.what();
			if (_response.getMessageMap().count(code) != 1)
				code = "500";
			handleERROR(code);
		}
	}
	// lseek(fd,0,SEEK_END);
	//create a response object and add it to responseMap
	// std::cerr << BLUE << "Remember and fix: Tam may not send response inside of cgi!!!" << RESET << std::endl;
	// this->_response.sendResponse(fd); // AE Tam may not send response inside of cgi
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
			#ifdef SHOW_LOG
				std::cerr << RED << "READING FROM FD " << fd << " FAILED" << std::endl;
			#endif
			throw Server::InternatServerErrorException();
		}
		else if (n == 0) // read reached eof
			break ;
		else // append one character from client request if it is an ascii-char
		{
			buffer[1] = '\0';
			if (!this->_isPrintableAscii(buffer[0]))
			{
				#ifdef SHOW_LOG
					std::cout << RED << "NON-ASCII CHAR FOUND IN REQUEST" << RESET << std::endl;
				#endif
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
			#ifdef SHOW_LOG
				std::cout << RED << "FIRST LINE TOO LONG" << RESET << std::endl;
			#endif
			throw Server::FirstLineTooLongException();
		}
	}
	if (charsRead <= MAX_REQUEST_HEADER_SIZE && this->_crlftwoFound() == true)
	{
		#ifdef SHOW_LOG
			std::cout << YELLOW << "Received->" << RESET << this->_requestHead << YELLOW << "<-Received on fd: " << fd << RESET << std::endl;
		#endif
	}
	else
	{
		#ifdef SHOW_LOG
			std::cout << RED << "HEAD BIGGER THAN " << MAX_REQUEST_HEADER_SIZE << " OR NO CRLFTWO FOUND (incomplete request)" << RESET << std::endl;
		#endif
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
	Cgi newCgi(request); // pass ConfigStruct here

	newCgi.printEnv();
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
