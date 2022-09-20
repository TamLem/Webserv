#include "Response.hpp"

/********** handle POST *********/

void Response::receiveChunk(int i)
{
	const int clientFd = i;
	size_t total;
	size_t bytesLeft;
	size_t bufferSize;

	if (this->_receiveMap[i].isChunked == true)
	{
		total = this->_handleChunked(i);
		bytesLeft = total;
		bufferSize = this->_receiveMap[i].bufferSize;
	}
	else
	{
		total = this->_receiveMap[i].total;
		bytesLeft = this->_receiveMap[i].bytesLeft;
		bufferSize = this->_receiveMap[i].bufferSize;
	}
	int n = 0;
	std::ofstream buffer;
	char chunk[bufferSize];
// opening the file and checking if its open
	buffer.open(this->_receiveMap[i].target.c_str(), std ::ios::out | std::ios_base::app | std::ios::binary);

	if (buffer.is_open() == false)
		throw Response::ERROR_423(); // check if this makes sense, maybe 500 is more appropriate ????????
// reading part
	else if (total > bufferSize && bytesLeft > bufferSize)
	{
		#ifdef SHOW_LOG_2
			std::cout << YELLOW << "bytesLeft: " << bytesLeft << RESET << std::endl;
		#endif
		n = read(clientFd, chunk, bufferSize);
	}
	else
	{
		#ifdef SHOW_LOG_2
			std::cout << YELLOW << "last bytesLeft: " << bytesLeft << RESET << std::endl;
		#endif
		n = read(clientFd, chunk, bytesLeft);
	}

	#ifdef SHOW_LOG_2
		std::cout << BOLD << GREEN << "Bytes received from " << clientFd << ": " << n << RESET << std::endl;
	#endif
// checking for errors of read
	if (n > 0)
	{
		bytesLeft -= n;
		#ifdef SHOW_LOG_2
			std::cout << BOLD << YELLOW << "Bytes left to read from " << clientFd << ": " << bytesLeft << RESET << std::endl;
		#endif
	}
	else if (n == -1)
	{
		#ifdef SHOW_LOG
			std::cout << RED << "reading for POST on fd " << clientFd << " failed" << RESET << std::endl;
		#endif
		this->_receiveMap.erase(i);
		bytesLeft = 0;
		buffer.close();
		throw Response::ClientDisconnect();
	}
// writing the read contents to the file
	// buffer[n] = '\0' // this would be needed for the next line
	// buffer << chunk; // this will stop writing if encounters a '\0', wich can happen in binary data!
	buffer.write(chunk, n); // with this there can even be a '\0' in there, it wont stop writing

// setting the bytesLeft and checking if all content was read
	if (bytesLeft)
	{
		this->_receiveMap[i].bytesLeft = bytesLeft;
		buffer.close();
	}
	else
	{
		buffer.close();
		this->_receiveMap.erase(i);
		this->_responseMap[clientFd].response = this->constructPostResponse();
		this->_responseMap[clientFd].total = this->_responseMap[clientFd].response.length();
		this->_responseMap[clientFd].bytesLeft = this->_responseMap[clientFd].response.length();
	}
}

// helper functions for POST

std::string Response::constructPostResponse() // this needs to be worked on !!!!!!
{
	std::ifstream postBody;
	postBody.open("./server/data/pages/post_success.html"); // do not do it like that maybe unsafe if file gets deleted, or build failsafety to keep it from failing if file does not exist

	std::stringstream buffer;
	buffer << "HTTP/1.1 201 Created";
	buffer << CRLFTWO;
	if (postBody.is_open())
		buffer << postBody.rdbuf();

	return (buffer.str());
}

bool Response::isInReceiveMap(int clientFd)
{
	return (this->_receiveMap.count(clientFd));
}

void Response::removeFromReceiveMap(int fd)
{
	if (this->_receiveMap.count(fd) == true)
		this->_receiveMap.erase(fd);
}

size_t Response::_handleChunked(int clientFd)
{
	// read the line until /r/n is read
	char *buffer = NULL;
	std::string hex = "";
	while (hex.find("\r\n") != std::string::npos)
	{
		if (read(clientFd, buffer, 1) < 1)
			throw Response::MissingChunkContentLengthException();
		hex.append(buffer);
	}
	hex.resize(hex.size() - 1);

	std::stringstream hexadecimal;
	hexadecimal << buffer;

	size_t length;
	hexadecimal >> std::hex >> length;

	return (length);
}

void Response::setPostTarget(int clientFd, std::string target)
{
	this->_receiveMap[clientFd].target = target;
	// std::stringstream buffer;
	// buffer << target << "." << clientFd << ".temp";
	// this->_receiveMap[clientFd].tempTarget = buffer.str();
}

void Response::checkPostTarget(int clientFd, const Request &request, int port)
{
	std::string target = request.getRoutedTarget();
	if (access(target.c_str(), F_OK ) != -1)
	{
		this->removeFromReceiveMap(clientFd);
		#ifdef SHOW_LOG_2
			std::stringstream message;
			message << "file " << target << " is already existing";
			LOG_RED(message.str());
		#endif
		this->_createFileExistingHeader(clientFd, request, port);
	}
}

static size_t _strToSizeT(std::string str)
{
	size_t out = 0;
	std::stringstream buffer;
	#ifdef __APPLE__
		buffer << SIZE_T_MAX;
	#else
		buffer << "18446744073709551615"; // size_t max value
	#endif
	std::string sizeTMax = buffer.str();
	if (str.find("-") != std::string::npos && str.find_first_of(DECIMAL) != std::string::npos && str.find("-") == str.find_first_of(DECIMAL) - 1)
	{
		std::cout << str << std::endl;
		throw Response::NegativeDecimalsNotAllowedException();
	}
	else if (str.find_first_of(DECIMAL) != std::string::npos)
	{
		std::string number = str.substr(str.find_first_of(DECIMAL));
		if (number.find_first_not_of(WHITESPACE) != std::string::npos)
			number = number.substr(0, number.find_first_not_of(DECIMAL));
		if (str.length() >= sizeTMax.length() && sizeTMax.compare(number) > 0)
		{
			std::cout << RED << ">" << number << RESET << std::endl;
			throw Response::SizeTOverflowException();
		}
		else
			std::istringstream(str) >> out;
	}
	return (out);
}

void Response::setPostChunked(int clientFd, std::string target, std::map<std::string, std::string> &headerFields) // don't do it with a temp file, just create it as normal file
{
	if (this->_receiveMap.count(clientFd) && headerFields.count("transfer-encoding") && headerFields["transfer-encoding"] == "chunked")
		this->_receiveMap[clientFd].isChunked = true;

	if (this->_receiveMap.count(clientFd) && this->_receiveMap[clientFd].isChunked == true)
	{
		std::stringstream fileName;
		fileName << target << "." << clientFd << ".temp"; // will result in a filename like: 7_larger.jpg.temp
		std::ofstream tempFile;
		tempFile.open(fileName.str(), std ::ios::out | std::ios_base::trunc | std::ios::binary);
		if (!tempFile.is_open())
		{
			LOG_RED("failed to create/open the temp file");
			throw Response::ERROR_423();
		}
		else
		{
			tempFile.close();
		}
	}
}

void Response::_createFileExistingHeader(int clientFd, const Request &request, int port)
{
	std::stringstream serverNamePort;
	serverNamePort << request.getHostName() << ":" << port; // creates ie. localhost:8080
// clear all the unused data
	this->headerFields.clear();
	this->body.clear();
	this->removeFromReceiveMap(clientFd);
	this->removeFromResponseMap(clientFd);
// set all the data for the header
	this->setProtocol(PROTOCOL);
	this->setStatus("303");
	std::string location = "http://" + serverNamePort.str() + request.getRawTarget();
	this->addHeaderField("location", location);

	// set fd to eof

	this->putToResponseMap(clientFd);
}

void Response::setPostLength(int clientFd, std::map<std::string, std::string> &headerFields)
{
	size_t length = 0;
	/* if (headerFields.count("Content-Length"))
	{
		std::cout << headerFields["Content-Length"] << std::endl;
		length = _strToSizeT(headerFields["Content-Length"]);
	}
	else  */if (headerFields.count("content-length"))
	{
		std::cout << headerFields["content-length"] << std::endl;
		length = _strToSizeT(headerFields["content-length"]);
	}

	this->_receiveMap[clientFd].total = length;
	this->_receiveMap[clientFd].bytesLeft = length;
}

void Response::setPostBufferSize(int clientFd, size_t bufferSize)
{
	this->_receiveMap[clientFd].bufferSize = bufferSize;
}

// bool Response::checkReceiveExistance(int clientFd)
// {
// 	if (this->_receiveMap.count(clientFd) == 1)
// 		return (true);
// 	else
// 		return (false);
// }
