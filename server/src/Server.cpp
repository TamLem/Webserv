#include "Server.hpp"
#include "Config.hpp"
#include "Cgi/Cgi.hpp"
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
		// if (numEvents == 0)
		// {
		// 	this->_socketHandler->removeInactiveClients();	// remove inactive clients
		// 	this->_response.clearResponseMap();
		// }
		for (int i = 0; i < numEvents; ++i)
		{
			#ifdef SHOW_LOG_2
				std::cout << "no. events: " << this->_socketHandler->getNumEvents() << " ev:" << i << std::endl;
			#endif
			this->_socketHandler->acceptConnection(i);
			// if (this->_socketHandler->removeClient(i) == true)
			// 	this->_response.removeFromResponseMap(this->_socketHandler->getFD(i));
			if (this->_socketHandler->readFromClient(i) == true)
			{
				#ifdef SHOW_LOG_2
				std::cout << BLUE << "read from client" << this->_socketHandler->getFD(i) << RESET << std::endl;
				#endif
				try
				{
					handleRequest(this->_socketHandler->getFD(i));
					this->_socketHandler->setWriteable(i);
				}
				catch(const std::exception& e)
				{
					std::cerr << YELLOW << e.what() << RESET << '\n';
					this->_socketHandler->removeClient(i, true);
					removeClientTraces(this->_socketHandler->getFD(i));
				}
			}
			else if (this->_socketHandler->writeToClient(i) == true)
			{
				#ifdef SHOW_LOG_2
				std::cout << BLUE << "write to client" << this->_socketHandler->getFD(i) << RESET << std::endl;
				#endif
				if (this->_response.sendRes(this->_socketHandler->getFD(i)) == true)
				{
					this->_socketHandler->removeClient(i, true);
					removeClientTraces(this->_socketHandler->getFD(i));
				}
			}
			if (this->_socketHandler->removeClient(i))
			{
				removeClientTraces(this->_socketHandler->getFD(i));
			} 	// remove inactive clients
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

void Server::handlePOST(int clientFd, const Request& request)
{
	#ifdef SHOW_LOG_2
		std::cout << "handlePost entered for" << clientFd << std::endl;
	#endif
	std::map<std::string, std::string> tempHeaderFields = request.getHeaderFields();
	std::stringstream clientMaxBodySize;
	clientMaxBodySize << this->_currentConfig.clientMaxBodySize;
	if (tempHeaderFields.count("Content-Length") == 0 && tempHeaderFields.count("content-length") == 0)
		throw Server::LengthRequiredException();
	else if (tempHeaderFields["Content-Length"] > clientMaxBodySize.str())
		throw Server::ContentTooLargeException();

	this->_response.setProtocol(PROTOCOL);
	this->_response.addHeaderField("Server", this->_currentConfig.serverName);
	this->_response.setStatus("201");
	this->_response.createBodyFromFile("./server/data/pages/post_test.html");


	// put this info into the receiveStruct!!!


	this->_response.setPostTarget(clientFd, request.getRoutedTarget()); // put target into the response class
	this->_response.setPostLength(clientFd, (request.getHeaderFields()));
	this->_response.setPostBufferSize(clientFd, this->_currentConfig.clientBodyBufferSize);
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
	std::map<std::string, std::string> headerFields = request.getHeaderFields();
	std::string host = headerFields["host"];
	this->_currentConfig = this->_config->getConfigStruct(host);
}

void Server::removeClientTraces(int clientFd)
{
	this->_response.removeFromReceiveMap(clientFd);
	this->_response.removeFromResponseMap(clientFd);
}

void Server::handleRequest(int fd) // i is the index from the evList of the socketHandler
{
	bool isCgi = false;
	this->_response.clear();
	this->loopDetected = false;
	try
	{
		// if (this->_response._receiveMap.count(fd) == 1)
		if (this->_response.checkReceiveExistance(fd) == 1)
			this->_response.receiveChunk(fd);
		else
		{
			this->_readRequestHead(fd); // read 1024 charackters or if less until /r/n/r/n is found
			Request request(this->_requestHead);
			this->applyCurrentConfig(request);
			//normalize target (in Request)
			//normalize target (in Request)
			//compression (merge slashes)
			//resolve relative paths
			//determine location
			request.setDecodedTarget(this->percentDecoding(request.getRawTarget()));
			request.setQuery(this->percentDecoding(request.getQuery()));
			isCgi = this->_isCgiRequest(this->_requestHead);
			#ifdef SHOW_LOG
				std::cout  << YELLOW << "URI after percent-decoding: " << request.getDecodedTarget() << std::endl;
			#endif
			this->matchLocation(request); // AE location with ü (first decode only unreserved chars?)
			// request.setRoutedTarget("." + request.getRoutedTarget());
			//check method
			checkLocationMethod(request);
			if (isCgi == true)
			{
				this->applyCurrentConfig(request);
				cgi_handle(request, fd, this->_currentConfig);
			}
			else if (request.getMethod() == "POST" || request.getMethod() == "PUT")
				handlePOST(fd, request);
			else if (request.getMethod() == "DELETE")
				handleDELETE(request);
			else
			{
				handleGET(request);
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
		if (this->_response.isInReceiveMap(fd) == true)
			this->_response.removeFromReceiveMap(fd);
		this->_response.putToResponseMap(fd);
	}
	// std::cerr << BLUE << "Remember and fix: Tam may not send response inside of cgi!!!" << RESET << std::endl;
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
			throw Server::ClientDisconnect();
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



void Server::cgi_handle(Request& request, int fd, ConfigStruct configStruct)
{

	int cgiPipe[2];
	if (pipe(cgiPipe) == -1)
	{
		#ifdef SHOW_LOG
			std::cerr << RED << "PIPE FAILED" << RESET << std::endl;
		#endif
		throw Server::InternalServerErrorException();
	}
	this->_socketHandler->setEvent(cgiPipe[1], EV_ADD | EV_CLEAR, EVFILT_READ);
	// this->_socketHandler->setEvent(cgiPipe[0], EVFILT_READ);
	this->_cgiSockets.push_back(cgiPipe[1]);
	
	Cgi newCgi(request, configStruct);
	#ifdef SHOW_LOG
		newCgi.printEnv();
	#endif

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
