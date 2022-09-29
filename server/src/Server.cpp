#include "Server.hpp"
#include "Config.hpp"
#include "Cgi/Cgi.hpp"
#include "Utils.hpp"
#include <dirent.h> // dirent, opendir
#include <cstdio> // remove




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
		std::cout << RED << "Server deconstructor called for " << this << RESET << std::endl;
	#endif
}

void Server::runEventLoop()
{
	while(keep_running)
	{

		#ifdef SHOW_LOG
			std::cout << "server running " << std::endl;
		#endif
		int numEvents = this->_socketHandler->getEvents();
		int clientFd = -1;
		// if (numEvents == 0)
		// {
			tryRemove:
			clientFd = this->_socketHandler->removeInactiveClients();	// remove inactive clients
			if (clientFd != -1 && this->_response.isInResponseMap(clientFd) == false)
			{
				this->_socketHandler->removeKeepAlive(clientFd);
				#ifdef SHOW_LOG
					std::stringstream message;
					message << "Client " << clientFd << " was timed-out";
					LOG_RED(message.str());
				#endif
				this->_response.clear();
				this->_response.setProtocol(PROTOCOL);
				this->_response.setStatus("408");
				this->_response.setRequestMethod("TIMEOUT");
				this->_response.addHeaderField("Connection", "close");
				this->_response.createErrorBody();
				this->_response.putToResponseMap(clientFd);
				this->_socketHandler->setEvent(clientFd, EV_ADD, EVFILT_WRITE);
				goto tryRemove;
			}
			#ifdef SHOW_LOG_2
			else if (clientFd != -1) // only to make the printed LOG_2 more beautiful
			{
				std::cout << "\033[A\033[K";
				std::cout << "\033[A\033[K";
			}
			#endif
		// }
		for (int i = 0; i < numEvents; ++i)
		{
			#ifdef SHOW_LOG_2
				std::cout << "event id:" << i << std::endl;
			#endif
			clientFd = this->_socketHandler->getFD(i);
			if (this->_socketHandler->acceptConnection(i))
			{
				this->_socketHandler->setTimeout(clientFd);
				continue ;
			}
			else if (this->_socketHandler->readFromClient(i) == true) /* && this->_response.isInResponseMap(this->_socketHandler->getFD(i)) == false */
			{
				#ifdef SHOW_LOG_2
					std::cout << BLUE << "read from client" << clientFd << RESET << std::endl;
				#endif
				this->_socketHandler->setTimeout(clientFd);
				try
				{
					handleRequest(clientFd);
					if (this->_response.isInReceiveMap(clientFd) == false)
					{
						this->_socketHandler->setWriteable(i);
						this->_socketHandler->setEvent(clientFd, EV_DELETE, EVFILT_READ); // removes any possible read event that is still left
					}
				}
				catch(const std::exception& e)
				{
					// this->_socketHandler->removeKeepAlive(clientFd); // already in remove client traces
					std::cerr << YELLOW << e.what() << RESET << '\n';
					this->_socketHandler->removeClient(i, true);
					removeClientTraces(clientFd);
				}
			}
			else if (this->_socketHandler->writeToClient(i) == true /* &&  !isCgiSocket(clientFd)   && this->_response.isInResponseMap(clientFd) */) // this commented part is a dirty workaround, only use it for testing!!!!
			{
				#ifdef SHOW_LOG_2
					std::cout << BLUE << "write to client" << clientFd << RESET << std::endl;
				#endif
				this->_socketHandler->setTimeout(clientFd);
				// if (this->_socketHandler->isKeepAlive(clientFd) == true) // tried to fix keep-alive connections!!! did not work, might be completely wrong!!!!
				// {
				// 	bool val = true;
				// 	setsockopt(clientFd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
				// 	LOG_GREEN("socket set to keepalive!!!!!");
				// }
				if (this->_socketHandler->isKeepAlive(clientFd) == true)
					this->_response.addHeaderField("Connection", "keep-alive");
				if (this->_cgiSockets.find(clientFd) != this->_cgiSockets.end())
				{
					this->_socketHandler->setEvent(clientFd, EV_DELETE, EVFILT_WRITE);
						// this->cgi_response_handle(clientFd);
				}
				else if (this->_response.sendRes(clientFd) == true)
				{
					// if (this->_response.was3XXCode(clientFd) == false)
					// 	this->_socketHandler->removeKeepAlive(clientFd);
					if (this->_socketHandler->isKeepAlive(clientFd) == true && this->_response.isInResponseMap(clientFd) == false)
						this->_socketHandler->setEvent(clientFd, EV_ADD, EVFILT_READ);
					if (this->_socketHandler->removeClient(i, true) == true)
					{
						removeClientTraces(clientFd);
					}
					else if (this->_response.isInResponseMap(clientFd) == false)
						this->_socketHandler->setEvent(clientFd, EV_DELETE, EVFILT_WRITE);
				}
			}
			else if (/* this->_response.isInReceiveMap(this->_socketHandler->getFD(i)) == 0 &&  */this->_socketHandler->removeClient(i) == true) // removes inactive clients ???? really?
			{
				removeClientTraces(clientFd);
			}
		}
	}
}

// bool Server::isCgiSocket(int clientFd)
// {
// 	for (std::map<int, FILE *>::iterator it = this->_cgiSockets.begin(); it != this->_cgiSockets.end(); ++it)
// 	{
// 		if (it->second == clientFd)
// 			return true;
// 	}
// 	return false;
// }


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
	_response.addHeaderField("server", this->_currentConfig.serverName);
	_response.addContentLengthHeaderField();
	_response.setStatus("200");
}

static size_t _strToSizeT(std::string str)
{
	size_t out = 0;
	std::stringstream buffer;
	#ifdef __APPLE__
		buffer << SIZE_T_MAX;
	#else
		buffer << "18446744073709551615";
	#endif
	std::string sizeTMax = buffer.str();
	if (str.find("-") != std::string::npos && str.find_first_of(DECIMAL) != std::string::npos && str.find("-") == str.find_first_of(DECIMAL) - 1)
	{
		std::cout << str << std::endl;
		throw SingleServerConfig::NegativeDecimalsNotAllowedException();
	}
	else if (str.find_first_of(DECIMAL) != std::string::npos)
	{
		std::string number = str.substr(str.find_first_of(DECIMAL));
		if (number.find_first_not_of(WHITESPACE) != std::string::npos)
			number = number.substr(0, number.find_first_not_of(DECIMAL));
		if (str.length() >= sizeTMax.length() && sizeTMax.compare(number) > 0)
		{
			std::cout << RED << ">" << number << RESET << std::endl;
			throw SingleServerConfig::SizeTOverflowException();
		}
		else
			std::istringstream(str) >> out;
	}
	return (out);
}

void Server::handlePOST(int clientFd, const Request& request)
{
	#ifdef SHOW_LOG_2
		std::cout << "handlePost entered for" << clientFd << std::endl;
	#endif
	std::map<std::string, std::string> tempHeaderFields = request.getHeaderFields();

	#ifdef FORTYTWO_TESTER
	if (request.getMethod() == "PUT")
	{
		this->_response.setProtocol(PROTOCOL);
		this->_response.addHeaderField("server", this->_currentConfig.serverName);
		// this->_response.addHeaderField("connection", "close");
		this->_response.setStatus("201");
		this->_response.setPostTarget(clientFd, request.getRoutedTarget()); // puts target into the response class
		this->_response.setPostBufferSize(clientFd, 100000);
		this->_response.setPostChunked(clientFd, /* request.getRoutedTarget(), */ tempHeaderFields);
		return ;
	}
	#endif

	if (tempHeaderFields.count("content-length") == 0)
		throw Server::LengthRequiredException();
	else if (tempHeaderFields.count("content-length") && _strToSizeT(tempHeaderFields["content-length"]) > this->_currentConfig.clientMaxBodySize)
		throw Server::ContentTooLargeException();

	this->_response.setProtocol(PROTOCOL);
	this->_response.addHeaderField("server", this->_currentConfig.serverName);
	// if (this->_socketHandler->isKeepAlive(clientFd) == true) // 201 is still missing the headerfields...!!!!!
	// 	this->_response.addHeaderField("Connection", "keep-alive");
	this->_response.setStatus("201");

	this->_response.createBodyFromFile("./server/data/pages/post_success.html");
	// put this info into the receiveStruct maybe ????

	this->_response.setPostTarget(clientFd, request.getRoutedTarget()); // puts target into the response class
	this->_response.setPostLength(clientFd, tempHeaderFields);
	this->_response.setPostBufferSize(clientFd, this->_currentConfig.clientBodyBufferSize);
	// this->_response.checkPostTarget(clientFd, request, this->_socketHandler->getPort(0));
	// if (this->_response.getStatus() == "303")
	// 	this->_socketHandler->removeKeepAlive(clientFd); // just for testing !!!!!!!!
	this->_response.setPostChunked(clientFd/* , request.getRoutedTarget() */, tempHeaderFields);
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
	_response.addHeaderField("server", this->_currentConfig.serverName);
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
	_response.addHeaderField("server", this->_currentConfig.serverName);
	_response.addContentLengthHeaderField();
}

void Server::applyCurrentConfig(const Request& request)
{
	std::map<std::string, std::string> headerFields = request.getHeaderFields();
	std::string host = headerFields["host"];
	this->_currentConfig = this->_config->getConfigStruct(host);
}

// removes any data of the client from _responseMap, _receiveMap and _keepalive
void Server::removeClientTraces(int clientFd)
{
	this->_response.removeFromReceiveMap(clientFd);
	this->_response.removeFromResponseMap(clientFd);
	this->_socketHandler->removeKeepAlive(clientFd);
}

// void Server::matchLocation(Request& request)
// {
// 	int max_count = 0;
// 	std::string target = request.getDecodedTarget();
// 	#ifdef SHOW_LOG_2
// 	std::cout  << RED << "target: " << target << std::endl;
// 	for (std::map<std::string, LocationStruct>::const_iterator it = this->_currentConfig.location.begin(); it != this->_currentConfig.location.end(); ++it)
// 	{
// 		std::cout << RED << it->first << ": "
// 		<< it->second.root << " is dir: " << it->second.isDir << RESET << "\n";
// 	}
// 	#endif
// 	for (std::map<std::string, LocationStruct>::const_iterator it = this->_currentConfig.location.begin(); it != this->_currentConfig.location.end(); ++it)
// 	{
// 		if (it->second.isDir == false)
// 		{
// 			if (routeFile(request, it, target) == 0)
// 				return ;
// 		}
// 		if (it->second.isDir == true)
// 			routeDir(request, it, target, max_count);
// 	}
// 	if (max_count == 0)
// 		routeDefault(request);
// 	#ifdef SHOW_LOG
// 		std::cout  << YELLOW << "DIR ROUTING RESULT!: " << request.getRoutedTarget() << " for location: " << _currentLocationKey  << std::endl;
// 	#endif
// }

// static std::string percentDecodingFix(std::string target)
// {
// 	std::string accent;
// 	accent += (const char)204;
// 	accent += (const char)136;

// 	std::string ü;
// 	ü += (const char)195;
// 	ü += (const char)188;

// 	std::string ä;
// 	ä += (const char)195;
// 	ä += (const char)164;

// 	std::string ö;
// 	ö += (const char)195;
// 	ö += (const char)182;

// 	target = staticReplaceInString(target, "u" + accent, ü);
// 	target = staticReplaceInString(target, "a" + accent, ä);
// 	target = staticReplaceInString(target, "o" + accent, ö);
// 	return (target);
// }

// std::string Server::percentDecoding(const std::string& str)
// {
// 	std::stringstream tmp;
// 	char c;
// 	int i = 0;
// 	while (str[i] != '\0')
// 	{
// 		if (str[i] == '%')
// 		{
// 			if (str[i + 1] == '\0' || str[i + 2] == '\0')
// 				throw InvalidHex();
// 			c = char(strtol(str.substr(i + 1, 2).c_str(), NULL, 16));
// 			if (c == 0)
// 				throw InvalidHex();
// 			tmp << c;
// 			i += 3;
// 		}
// 		else
// 		{
// 			tmp << str[i];
// 			i++;
// 		}
// 	}
// 	return (percentDecodingFix(tmp.str()));
// }

// void Server::checkLocationMethod(const Request& request) const
// {
// 	if (this->_currentLocationKey.empty() == true)
// 		return ;
// 	if (this->_currentConfig.location.find(_currentLocationKey)->second.allowedMethods.count(request.getMethod()) != 1)
// 		throw MethodNotAllowed();
// }

// bool Server::_isCgiRequest(std::string requestHead) // AE this function ahs to be included in locationMatching
// {
// 	requestHead = requestHead.substr(0, requestHead.find("HTTP/1.1")); // AE is formatting checked before?
// 	if (requestHead.find("/cgi/") != std::string::npos) // AE file extension should deterime if something is cgi or not
// 		return (true);
// 	if (this->_currentConfig.cgiBin.length() != 0 && requestHead.find(this->_currentConfig.cgiBin) != std::string::npos)
// 		return (true);
// 	else if (this->_currentConfig.cgi.size() != 0)
// 	{
// 		std::map<std::string, std::string>::const_iterator it = this->_currentConfig.cgi.begin();
// 		for (; it != this->_currentConfig.cgi.end(); ++it)
// 		{
// 			if (requestHead.find(it->first) != std::string::npos) // AE define this better (has to be ending, not just existing)
// 				return (true);
// 		}
// 	}
// 	return (false);
// }

void Server::handleRequest(int clientFd) // i is the index from the evList of the socketHandler
{
	bool isCgi = false;
	this->_response.clear();
	this->loopDetected = false;
	try
	{
		if (this->_response.isInReceiveMap(clientFd) == true)
			this->_response.receiveChunk(clientFd);
		else
		{
			this->_readRequestHead(clientFd); // read 1024 charackters or if less until /r/n/r/n is found
			Request request(this->_requestHead);
			this->_response.setRequestMethod(request.getMethod());
			this->applyCurrentConfig(request);
			//normalize target (in Request)
			//normalize target (in Request)
			//compression (merge slashes)
			//resolve relative paths
			//determine location
			request.setDecodedTarget(percentDecoding(request.getRawTarget()));
			request.setQuery(percentDecoding(request.getQuery()));
			isCgi = this->_isCgiRequest(this->_requestHead);
			#ifdef SHOW_LOG
				std::cout  << YELLOW << "URI after percent-decoding: " << request.getDecodedTarget() << std::endl;
			#endif
			this->matchLocation(request); // AE location with ü (first decode only unreserved chars?)
			// request.setRoutedTarget("." + request.getRoutedTarget());
			//check method
			if (isCgi == false)
				this->checkLocationMethod(request);
			if (request.getHeaderFields().count("connection") && request.getHeaderFields().find("connection")->second == "keep-alive")
				this->_socketHandler->addKeepAlive(clientFd);

			if (isCgi == true)
			{
				this->applyCurrentConfig(request);
				cgi_handle(request, clientFd, this->_currentConfig);
			}
			else if (request.getMethod() == "POST" || request.getMethod() == "PUT")
				this->handlePOST(clientFd, request);
			else if (request.getMethod() == "DELETE")
				this->handleDELETE(request);
			else
			{
				this->handleGET(request);
			}
		}
	}
	catch (std::exception& exception)
	{
		std::cout << RED << "Exception: " << exception.what() << std::endl;
		std::string code = exception.what();
		if (std::string(exception.what()) == "client disconnect")
		{
			throw exception;
		}
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
		if (this->_response.isInReceiveMap(clientFd) == true)
			this->_response.removeFromReceiveMap(clientFd);
		this->_response.putToResponseMap(clientFd);
	}
	// std::cerr << BLUE << "Remember and fix: Tam may not send response inside of cgi!!!" << RESET << std::endl;
}



// read 1024 charackters or if less until /r/n/r/n is found
void Server::_readRequestHead(int clientFd)
{
	this->_requestHead.clear();
	size_t charsRead = 0;
	bool firstLineBreak = false;
	int n = 0;
	char buffer[2];
	while (charsRead < MAX_REQUEST_HEADER_SIZE)
	{
		n = read(clientFd, buffer, 1);
		if (buffer[0] == '\n' && n < 0)
		{
			#ifdef SHOW_LOG_2
				LOG_RED("Incomplete Request detected");
			#endif
			throw Server::BadRequestException();
		}
		else if (n < 0) // read had an error reading from fd, was failing PUT
		{
			#ifdef SHOW_LOG
				std::cerr << RED << "READING FROM FD " << clientFd << " FAILED" << std::endl;
			#endif
			#ifndef FORTYTWO_TESTER
				throw Server::ClientDisconnect(); // only for testing!!!!!
			#endif
		}
		else if (n == 0) // read reached eof
			break ;
		else // append one character from client request if it is an ascii-char
		{
			buffer[1] = '\0';
			if (!_isPrintableAscii(buffer[0]))
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
		if (firstLineBreak == false && charsRead > MAX_REQUEST_LINE_SIZE)
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
			std::cout << YELLOW << "Received->" << RESET << this->_requestHead << YELLOW << "<-Received on fd: " << clientFd << RESET << std::endl;
		#endif
	}
	else
	{
		#ifdef SHOW_LOG
			std::cout << RED << "HEAD BIGGER THAN " << MAX_REQUEST_HEADER_SIZE << " OR NO CRLFTWO FOUND (incomplete request)" << RESET << std::endl;
		#endif
		std:: cout << YELLOW << "received >" << RESET << this->_requestHead << YELLOW << "<" << RESET << std::endl;
		throw Server::BadRequestException();
	}
}

void Server::cgi_handle(Request& request, int fd, ConfigStruct configStruct)
{
// 	#ifdef FORTYTWO_TESTER
// // was missing for the tester maybe????
// 	_response.setProtocol(PROTOCOL);
// 	_response.setStatus("200");
//  //
// 	#endif
	// int cgiPipe[2];
	// if (pipe(cgiPipe) == -1)
	// {
	// 	#ifdef SHOW_LOG
	// 		std::cerr << RED << "PIPE FAILED" << RESET << std::endl;
	// 	#endif
	// 	throw Server::InternalServerErrorException();
	// }
	// cout << "cgi out pipe: " << cgiPipe[0] << endl;
	// dup2(fd, cgiPipe[0]);
	// this->_socketHandler->setEvent(cgiPipe[0], EV_ADD, EVFILT_READ);
	// this->_socketHandler->setEvent(cgiPipe[0], EVFILT_READ);
	FILE *tempFile = tmpfile();
	this->_cgiSockets[fd] = tempFile;
	int cgi_out = fileno(tempFile);
	// this->_socketHandler->_cgiSockets.push_back(cgiPipe[0]);
	Cgi newCgi(request, configStruct);
	#ifdef SHOW_LOG
		newCgi.printEnv();
	#endif
	newCgi.init_cgi(fd, cgi_out);
	// close(cgi_out);
	cgi_response_handle(fd);
	/* 
		if (request.isChunked() == )
	*/
	// if (request.getHeaderFields().count("transfer-encoding") 
	// 	&& request.getHeaderFields().find("transfer-encoding")->second == "chunked")
	// {
		
	// }


	//initiate cgi response
	//listen to event on cgiPipe[0]
	//if event is read, read from cgiPipe[0] and write to fd
	//if event is write, write to cgiPipe[1]
	//if event is error, close cgiPipe[0] and cgiPipe[1]
	//if event is hangup, close cgiPipe[0] and cgiPipe[1]
	//if event is timeout, close cgiPipe[0] and cgiPipe[1]
	//if event is terminate, close cgiPipe[0] and cgiPipe[1]
	//if event is close, close cgiPipe[0] and cgiPipe[1]


	// newCgi.cgi_response(fd);
}

void Server::cgi_response_handle(int clientFd)
{
	FILE *tempFile = this->_cgiSockets[clientFd];
	long	lSize = ftell(tempFile);
	cout << "lSize: " << lSize << endl;
	rewind(tempFile);
	int cgi_out = fileno(tempFile);
	
	CgiResponse response(cgi_out, clientFd);
	
	response.parseCgiHeaders();
	response.sendResponse();
}

	// char buffer[lSize];
	// int n = 0;
	// if((n = fread(buffer, 1, lSize - 1, tempFile)) > 0 || !ferror(tempFile))
	// {
	// 	cout << "cgi output: " << buffer << endl;
	// 	write(clientFd, buffer, n);
	// 	buffer[n] = '\0';
	// 	exit(0);
	// }
	// else
	// {
	// 	perror("fread");
	// 	cout << "no cgi out" << endl;
	// }
	// return ;
	// cout << "cgi response handle" << endl;