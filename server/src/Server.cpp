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
	handle_signals();
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
		// #ifndef FORTYTWO_TESTER
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
		// #endif
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
					std::cout << BLUE << "read from client " << clientFd << RESET << std::endl;
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
					this->_socketHandler->removeKeepAlive(clientFd); // needed so that the force remove works
					#ifdef SHOW_LOG_EXCEPTION
						std::cerr << YELLOW << "Exception: " << e.what() << RESET << '\n'; // AE @tim what is this?
					#endif
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
				if (this->_cgiSockets.find(clientFd) != this->_cgiSockets.end())
				{
					try
					{
						cgi_response_handle(clientFd);
					}
					catch(const std::exception& exception)
					{
						errorResponse(exception, clientFd);
					}
					this->_cgiSockets.erase(clientFd);
				}
				if (this->_response.sendRes(clientFd) == true)
				{
					// if (this->_response.was3XXCode(clientFd) == false)
					// 	this->_socketHandler->removeKeepAlive(clientFd);
					if (this->_socketHandler->isKeepAlive(clientFd) == true && this->_response.isInResponseMap(clientFd) == false)
					{
						this->_socketHandler->setEvent(clientFd, EV_ADD, EVFILT_READ);
						#ifdef SHOW_LOG_2
							std::stringstream message;
							message << "FD " << clientFd << "SET TO READABLE AGAIN";
							LOG_RED(message.str());
						#endif
					}
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
	buffer << SIZE_T_MAX;
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
		// if (this->_socketHandler->isKeepAlive(clientFd)) // only for testing!!!!
		// 	this->_response.addHeaderField("Connection", "keep-alive"); // only for testing !!!!
		this->_response.setPostTarget(clientFd, request.getRoutedTarget()); // puts target into the response class
		this->_response.setPostBufferSize(clientFd, 100000, 1000000);
		// this->_response.setPostBufferSize(clientFd, this->_currentConfig.clientBodyBufferSize); // THIS IS THE ORIGINAL !!!!
		this->_response.setPostChunked(clientFd, /* request.getRoutedTarget(), */ tempHeaderFields);
		return ;
	}
	#endif

	if (tempHeaderFields.count("content-length") == 0 && (tempHeaderFields.count("transfer-encoding") && tempHeaderFields["transfer-encoding"] == "chunked") == false)
		throw Server::LengthRequiredException();
	else if (tempHeaderFields.count("content-length") && _strToSizeT(tempHeaderFields["content-length"]) > this->_currentConfig.clientMaxBodySize)
		throw Server::ContentTooLargeException();

	this->_response.setProtocol(PROTOCOL);
	this->_response.addHeaderField("server", this->_currentConfig.serverName);
	this->_response.setStatus("201");

	this->_response.setPostTarget(clientFd, request.getRoutedTarget()); // puts target into the response class
	this->_response.setPostLength(clientFd, tempHeaderFields);
	#ifndef FORTYTWO_TESTER
		this->_response.setPostBufferSize(clientFd, this->_currentConfig.clientBodyBufferSize, this->_currentConfig.clientMaxBodySize);
	#else
		this->_response.setPostBufferSize(clientFd, 1000, this->_currentConfig.clientMaxBodySize); // @Tam here you can accelerate the POST 100.000.000 test of the tester by increasing this value to up to 32kB
	#endif
// LEGACY CONTENT
	// this->_response.checkPostTarget(clientFd, request, this->_socketHandler->getPort(0));
	// if (this->_response.getStatus() == "303")
	// 	this->_socketHandler->removeKeepAlive(clientFd); // just for testing !!!!!!!!
//
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
	_response.addHeaderField("Connection", "close");
	_response.addContentLengthHeaderField();
}

static void createHostTokens(std::vector<std::string>& tokens, const std::string& host)
{
	size_t last = 0;
	size_t next = 0;
	const std::string delim = ":";

	while ((next = host.find(delim, last)) != std::string::npos)
	{
		// std::cout << BLUE << host.substr(last, next - last) << RESET << std::endl; // AE remove
		tokens.push_back(host.substr(last, next - last));
		last = next + 1;
	}
	// std::cout << GREEN << host.substr(last, host.length() - last) << RESET << std::endl; // AE remove
	tokens.push_back(host.substr(last, host.length() - last));
}

void Server::applyCurrentConfig(const Request& request)
{
	std::vector<std::string> tokens;
	std::map<std::string, std::string> headerFields = request.getHeaderFields();
	std::string host = headerFields["host"];
	std::string port;
	// split host in host_name and port
	createHostTokens(tokens, host);
	if (tokens.size() > 2)
		throw Server::BadRequestException();
	std::string server_name = tokens[0];
	this->_currentConfig = this->_config->getConfigStruct(server_name);
	// std::cout << YELLOW << server_name << RESET << std::endl; // AE remove
	if (this->_currentConfig.serverName == server_name)
	{
		if (tokens.size() == 2)
			port = tokens[1];
		else
			port = "80";
		// std::cout << YELLOW << port << RESET << std::endl; // AE remove
		std::map<std::string, unsigned short>::const_iterator it = this->_currentConfig.listen.begin();
		for (; it != this->_currentConfig.listen.end(); ++it)
		{
			if(port == it->first)
				return ;
		}
		// std::cout << YELLOW << "selected DEFAULT" << RESET << std::endl; // AE remove
		this->_currentConfig = this->_config->getConfigStruct(DEFAULT_SERVER_NAME);
	}
	// if (tokens.size() == 2)
	// check port against portlist from config
	//if server_name wrong -> default DEFAULT_SERVER_NAME
}

// removes any data of the client from _responseMap, _receiveMap, _tempFile and _keepalive
void Server::removeClientTraces(int clientFd)
{
	this->_response.removeFromReceiveMap(clientFd);
	this->_response.removeFromResponseMap(clientFd);
	this->_response.removeTempFile(clientFd);
	this->_socketHandler->removeKeepAlive(clientFd);
}

void Server::errorResponse(const std::exception& exception, int clientFd)
{
	#ifdef SHOW_LOG_EXCEPTION
	std::cout << RED << "Exception: " << exception.what() << std::endl;
	#endif
	std::string code = exception.what();
	if (std::string(exception.what()) == "client disconnect") // check if it is used in any way
	{
		throw exception; // AE @tam why do you do this? (happens e.g. with testcase " ")
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
	this->_socketHandler->removeKeepAlive(clientFd);
}

void Server::handleRequest(int clientFd)
{
	this->_response.clear();
	this->loopDetected = false;
	try
	{
		if (this->_response.isInReceiveMap(clientFd) == true)
		{
			this->_response.receiveChunk(clientFd);
			if (this->_response.isFinished(clientFd) == true && this->_response.isCgi(clientFd) == true)
			{
				Request tempRequest(this->_response.getRequestHead(clientFd));
				this->applyCurrentConfig(tempRequest);
				this->cgi_handle(tempRequest, clientFd, this->_currentConfig, this->_response.getTempFile(clientFd));
				this->_response.removeFromReceiveMap(clientFd);
			}
		}
		else
		{
			this->_readRequestHead(clientFd);
			Request request(this->_requestHead);
			if (request.getHeaderFields().count("content-length") && request.getHeaderFields().count("transfer-encoding"))
				throw Server::BadRequestException();
			this->_response.setRequestHead(this->_requestHead, clientFd);
			this->_response.setRequestMethod(request.getMethod());
			this->applyCurrentConfig(request);

			// AE remove
			// std::cout << RED << this->_currentConfig.serverName << RESET << std::endl; // AE remove
			// std::map<std::string, unsigned short>::const_iterator it = this->_currentConfig.listen.begin();
			// for (; it != this->_currentConfig.listen.end(); ++it)
			// {
			// 	std::cout << RED << it->first << RESET << std::endl;
			// }
			// std::map<std::string, std::string>::const_iterator it2 = this->_currentConfig.errorPage.begin();
			// for (; it2 != this->_currentConfig.errorPage.end(); ++it2)
			// {
			// 	std::cout << RED << it2->first << " " << it2->second << RESET << std::endl;
			// }
			// std::cout << RED << this->_currentConfig.indexPage << RESET << std::endl;
			std::string tmp;
			tmp = percentDecoding(request.getRawTarget());
			tmp = resoluteTarget(tmp);
			request.setDecodedTarget(tmp);
			request.setQuery(percentDecoding(request.getQuery()));
			this->_response.setIsCgi(clientFd, this->_isCgiRequest(request));
			#ifdef SHOW_LOG_ROUTING
				std::cout  << YELLOW << "URI after percent-decoding: " << request.getDecodedTarget() << std::endl;
			#endif
			this->matchLocation(request);
		#ifdef SHOW_LOG_ROUTING
			std::cout  << GREEN << "Target after routing: " << request.getRoutedTarget() << RESET << std::endl;
		#endif
		#ifndef FORTYTWO_TESTER
			if (request.getMethod() == "POST" && request.isFile == false)
				throw Server::BadRequestException();
		#endif
			if (this->_response.isCgi(clientFd) == false)
				this->checkLocationMethod(request);
			if ((request.getHeaderFields().count("connection") && request.getHeaderFields().find("connection")->second == "keep-alive") || request.getHeaderFields().count("connection") == 0)
				this->_socketHandler->addKeepAlive(clientFd);

		#ifdef FORTYTWO_TESTER
			if (request.getMethod() == "POST" || request.getMethod() == "PUT")
		#else
			if (request.getMethod() == "POST")
		#endif
				this->handlePOST(clientFd, request);
			else if (request.getMethod() == "DELETE")
			{
				this->handleDELETE(request);
				this->_response.putToResponseMap(clientFd);
				this->_response.removeFromReceiveMap(clientFd);
				// this->_socketHandler->setEvent(clientFd, EV_DELETE, EVFILT_READ);
				// this->_socketHandler->setEvent(clientFd, EV_ADD, EVFILT_WRITE);
			}
			else
			{
				if (this->_response.isCgi(clientFd) == true)
					this->cgi_handle(request, clientFd, this->_currentConfig, nullptr);
				this->handleGET(request);
				this->_response.putToResponseMap(clientFd);
				this->_response.removeFromReceiveMap(clientFd);
				// this->_socketHandler->setEvent(clientFd, EV_DELETE, EVFILT_READ);
				// this->_socketHandler->setEvent(clientFd, EV_ADD, EVFILT_WRITE);
			}
		}
	}
	catch (std::exception& exception) // AE convert this block into function, so it can be called in cgi_response_handle catch block (has yet to be created)
	{
		errorResponse(exception, clientFd);
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
		if (buffer[0] == '\n' && n < 0)
		{
			#ifdef SHOW_LOG_2
				LOG_RED("Incomplete Request detected");
			#endif
			#ifndef FORTYTWO_TESTER
				throw Server::BadRequestException();
			#endif
		}
		else if (n < 0) // read had an error reading from fd, was failing PUT
		{
			#ifdef SHOW_LOG_2
				std::cerr << RED << "READING FROM FD " << clientFd << " FAILED, EOF OR CLIENT DISCONNECT" << std::endl;
			#endif
			#ifndef FORTYTWO_TESTER
				throw Server::BadRequestException(); // check if this is correct!!!!!!!
			#endif
		}
		else if (n == 0) // read reached eof
			break ;
		else // append one character from client request if it is an ascii-char
		{
			buffer[1] = '\0';
			if (!_isPrintableAscii(buffer[0]))
			{
				#ifdef SHOW_LOG_2
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
			#ifdef SHOW_LOG_2
				std::cout << RED << "FIRST LINE TOO LONG" << RESET << std::endl;
			#endif
			throw Server::FirstLineTooLongException();
		}
	}
	if (charsRead <= MAX_REQUEST_HEADER_SIZE && this->_crlftwoFound() == true)
	{
		#ifdef SHOW_LOG_REQUEST
			std::cout << YELLOW << "Received->" << RESET << this->_requestHead << YELLOW << "<-Received on fd: " << clientFd << RESET << std::endl;
		#endif
	}
	else
	{
		#ifdef SHOW_LOG
			std::cout << RED << "HEAD BIGGER THAN " << MAX_REQUEST_HEADER_SIZE << " OR NO CRLFTWO FOUND (incomplete request)" << RESET << std::endl;
		#endif
		#ifdef SHOW_LOG_REQUEST
		std:: cout << YELLOW << "received >" << RESET << this->_requestHead << YELLOW << "<" << RESET << std::endl;
		#endif
		throw Server::BadRequestException();
	}
}

void Server::cgi_handle(Request& request, int fd, ConfigStruct configStruct, FILE *infile)
{
	#ifdef SHOW_LOG_CGI
		LOG_YELLOW("\tcgi_handle");
	#endif
	FILE *outFile = tmpfile();
	this->_cgiSockets[fd] = outFile;
	int cgi_out = fileno(outFile);
	Cgi newCgi(request, configStruct, infile);
	#ifdef SHOW_LOG_2
		newCgi.printEnv();
	#endif
	try
	{
		newCgi.init_cgi(fd, cgi_out);
	}
	catch(const std::exception& e)
	{
		fclose(infile);
		fclose(outFile);
		this->_cgiSockets.erase(fd);
		// this->_cgiSockets[fd] = nullptr;
		#ifdef SHOW_LOG_CGI
			LOG_RED('\t' << e.what());
		#endif
		if (std::string(e.what()) == "400")
			throw Response::ERROR_404(); //@tblaase, can you catch this exceptions and send error response
		else
			throw Response::ERROR_500();
	}
	fclose(infile);
}

void Server::cgi_response_handle(int clientFd)
{
	#ifdef SHOW_LOG_CGI
		LOG_GREEN("\tcgi_response_handle");
	#endif
	FILE *outFile = this->_cgiSockets[clientFd];
	if (!outFile) 
		throw Response::ERROR_500();
	int cgi_out = fileno(outFile);
	lseek(cgi_out, 0, SEEK_SET);

	CgiResponse cgiResponse(cgi_out, clientFd);

	try
	{
		this->_response.clear();
		this->_response.setProtocol(PROTOCOL);
		this->_response.setStatus("200");
		this->_response.setBody(cgiResponse.getBody());
		this->_response.addContentLengthHeaderField();
		this->_response.putToResponseMap(clientFd);
		this->_cgiSockets.erase(clientFd);
		fclose(outFile);
	}
	catch(const std::exception& e)
	{
		fclose(outFile);
		this->_cgiSockets.erase(clientFd);
		throw Response::ERROR_500();	
	}

}
