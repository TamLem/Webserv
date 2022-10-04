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
		int clientFd = -1;
		if (numEvents == 0)
		{
			tryRemove:
			clientFd = this->_socketHandler->removeInactiveClients();	// remove inactive clients
			if (clientFd != -1)
			{
				this->removeClientTraces(clientFd);
				#ifdef SHOW_LOG
					std::cout << RED << "Client " << clientFd << " was timed-out" << RESET << std::endl;
				#endif
				close(clientFd);
				goto tryRemove;
			}
		}
		for (int i = 0; i < numEvents; ++i)
		{
			clientFd = this->_socketHandler->getFD(i);
			#ifdef SHOW_LOG_2
				std::cout << "no. events: " << numEvents << " ev:" << i << std::endl;
			#endif
			if (this->_socketHandler->acceptConnection(i))
			{
				this->_socketHandler->setTimeout(clientFd);
				continue ;
			}
			else if (this->_socketHandler->readFromClient(i) == true /* && this->_response.isInResponseMap(this->_socketHandler->getFD(i)) == false */)
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
			else if (this->_socketHandler->writeToClient(i) == true /* && this->_response.isInResponseMap(clientFd) */) // this commented part is a dirty workaround, only use it for testing!!!!
			{
				#ifdef SHOW_LOG_2
					std::cout << BLUE << "write to client" << clientFd << RESET << std::endl;
				#endif
				this->_socketHandler->setTimeout(clientFd);
				if (this->_response.sendRes(clientFd) == true)
				{
					if (this->_response.was3XXCode(clientFd) == false)
						this->_socketHandler->removeKeepAlive(clientFd);
					// else
					// 	this->_socketHandler->setEvent(this->_socketHandler->getFD(i), EV_ADD, EVFILT_READ); // tried to reuse the same socket for the keepalive answer, did not work for some reason.... try implementing the timeout we talked about instead
					if (this->_socketHandler->removeClient(i, true) == true)
					{
						removeClientTraces(clientFd);
					}
					else if (this->_response.isInResponseMap(clientFd) == false)
						this->_socketHandler->setEvent(clientFd, EV_DELETE, EVFILT_WRITE);
				}
			}
			if (/* this->_response.isInReceiveMap(this->_socketHandler->getFD(i)) == 0 &&  */this->_socketHandler->removeClient(i) == true) // removes inactive clients ???? really?
			{
				// close(clientFd); // check if it breaks anything
				removeClientTraces(clientFd);
			}
		}
	}
}


void Server::handleGET(const Request& request)
{
	_response.setProtocol(PROTOCOL);
	// if (request.isFile == false && targetExists(request.getRoutedTarget() + request.indexPage) == false)
	if (request.isFile == false)
	{
		if (targetExists(request.getRoutedTarget() + request.indexPage) == false)
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
	}
	else
		_response.createBodyFromFile(request.getRoutedTarget());
		// _response.createBodyFromFile(request.getRoutedTarget() + request.indexPage);
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
		this->_response.setPostChunked(clientFd, request.getRoutedTarget(), tempHeaderFields);
		return ;
	}
	#endif

	if (tempHeaderFields.count("content-length") == 0)
		throw Server::LengthRequiredException();
	else if (tempHeaderFields.count("content-length") && _strToSizeT(tempHeaderFields["content-length"]) > this->_currentConfig.clientMaxBodySize)
		throw Server::ContentTooLargeException();

	this->_response.setProtocol(PROTOCOL);
	this->_response.addHeaderField("server", this->_currentConfig.serverName);
	this->_response.setStatus("201");

	this->_response.createBodyFromFile("./server/data/pages/post_success.html");
	// put this info into the receiveStruct maybe ????

	this->_response.setPostTarget(clientFd, request.getRoutedTarget()); // puts target into the response class
	this->_response.setPostLength(clientFd, tempHeaderFields);
	this->_response.setPostBufferSize(clientFd, this->_currentConfig.clientBodyBufferSize);
	this->_response.checkPostTarget(clientFd, request, this->_socketHandler->getPort(0));
	this->_response.setPostChunked(clientFd, request.getRoutedTarget(), tempHeaderFields); // this should set bool to true and create the tempTarget
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
	_response.setStatus("204");
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
			this->_readRequestHead(clientFd);
			Request request(this->_requestHead);
			this->_response.setRequestMethod(request.getMethod());
			this->applyCurrentConfig(request);
			request.setDecodedTarget(this->percentDecoding(request.getRawTarget()));
			request.setQuery(this->percentDecoding(request.getQuery()));
			isCgi = this->_isCgiRequest(this->_requestHead);
			#ifdef SHOW_LOG
				std::cout  << YELLOW << "URI after percent-decoding: " << request.getDecodedTarget() << std::endl;
			#endif
			this->matchLocation(request);
			if (isCgi == false)
				checkLocationMethod(request);

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
		if (n < 0) // read had an error reading from fd, was failing PUT
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
	#ifdef FORTYTWO_TESTER
// was missing for the tester maybe????
	_response.setProtocol(PROTOCOL);
	_response.setStatus("200");
 //
	#endif
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
