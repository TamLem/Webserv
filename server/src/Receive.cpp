#include "Response.hpp"

/********** handle POST *********/

/**
 * @brief  Will read a chunk of data from a POST request and write it to the appropriate file
 * @note  will work on chunked and not chunked requests
 * @param  i: clients filedescriptor
 * @retval None
 */
void Response::receiveChunk(int i)
{
	const int clientFd = i;
	size_t total;
	size_t bytesLeft;
	size_t bufferSize;

// for the case where it is chunked by the client
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

/**
 * @brief  this will create the responses content for a successfull POST
 * @note
 * @retval the response as a string with head and body that needs to be sent to the client
 */
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

/**
 * @brief  checks if a clients filedescriptor is already part of our receiveMap
 * @note
 * @param  clientFd: clients filedescriptor
 * @retval bool of the state
 */
bool Response::isInReceiveMap(int clientFd)
{
	return (this->_receiveMap.count(clientFd));
}

/**
 * @brief  will remove a client from the receiveMap if existant
 * @note
 * @param  fd: clients filedescriptor
 * @retval None
 */
void Response::removeFromReceiveMap(int fd)
{
	if (this->_receiveMap.count(fd) == true)
		this->_receiveMap.erase(fd);
}

/**
 * @brief  read the first line of a chunked message including the \r\n and translate the hexadecimal number to size_t
 * @note
 * @param  clientFd: clients filedescriptor
 * @retval the translated to size_t hexadecimal
 */
size_t Response::_handleChunked(int clientFd)
{
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

/**
 * @brief  will set the target for the post
 * @note   was previously also setting the temp file, might be used again
 * @param  clientFd: clients filedescriptor
 * @param  target: the target specified in the request, but with the routed path
 * @retval None
 */
void Response::setPostTarget(int clientFd, std::string target)
{
	this->_receiveMap[clientFd].target = target;
	// std::stringstream buffer;
	// buffer << target << "." << clientFd << ".temp";
	// this->_receiveMap[clientFd].tempTarget = buffer.str();
}

/**
 * @brief  checks existance of the target specified by the request
 * @note   POST can not write to a file that already is existing
 * @param  clientFd: clients filedescriptor
 * @param  &request: the request object
 * @param  port: the port that was used for the request
 * @retval None
 */
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

/**
 * @brief  converts a string to a size_t, similar to atoi
 * @note
 * @param  str: the string that contains the number you want to convert
 * @retval the found number or an exception
 */
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

/**
 * @brief  checks if a POST request is chunked or not
 * @note
 * @param  clientFd: clients filedescriptor
 * @param  target: the target specified by the request, is outdated because temp files are currently not in use
 * @param  &headerFields: the header fields of the request
 * @retval None
 */
void Response::setPostChunked(int clientFd/* , std::string target */, std::map<std::string, std::string> &headerFields) // don't do it with a temp file, just create it as normal file
{
	if (this->_receiveMap.count(clientFd) && headerFields.count("transfer-encoding") && headerFields["transfer-encoding"] == "chunked")
		this->_receiveMap[clientFd].isChunked = true;

	// if (this->_receiveMap.count(clientFd) && this->_receiveMap[clientFd].isChunked == true)
	// {
	// 	std::stringstream fileName;
	// 	fileName << target << "." << clientFd << ".temp"; // will result in a filename like: 7_larger.jpg.temp
	// 	std::ofstream tempFile;
	// 	tempFile.open(fileName.str(), std ::ios::out | std::ios_base::trunc | std::ios::binary);
	// 	if (!tempFile.is_open())
	// 	{
	// 		LOG_RED("failed to create/open the temp file");
	// 		throw Response::ERROR_423();
	// 	}
	// 	else
	// 	{
	// 		tempFile.close();
	// 	}
	// }
}

/**
 * @brief  creates a 303 response header for POST requests if the file is already existing
 * @note
 * @param  clientFd: clients filedescriptor
 * @param  &request: the request object
 * @param  port: the port on which the request came
 * @retval None
 */
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
	// this->addHeaderField("Connection", "close");

	this->putToResponseMap(clientFd);
}

/**
 * @brief  sets the total and bytesleft for the request, only for non chunked requests
 * @note
 * @param  clientFd: clients filedescriptor
 * @param  &headerFields: header fields of the request
 * @retval None
 */
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

/**
 * @brief  sets the buffersize defined in the config file for each POST request
 * @note
 * @param  clientFd: clients filedescriptor
 * @param  bufferSize: buffer size specified in the config
 * @retval None
 */
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
