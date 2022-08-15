#include "Response.hpp"

/********** handle POST *********/

void Response::receiveChunk(int i)
{
	if (this->_receiveMap[i].isChunked == true)
	{
		this->_fillTempFile(i);
	}
	else
	{
		const int clientFd = i;
		size_t total = this->_receiveMap[i].total;
		size_t bytesLeft = this->_receiveMap[i].bytesLeft;
		size_t bufferSize = this->_receiveMap[i].bufferSize;

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
}

// helper functions for POST

std::string Response::constructPostResponse() // this needs to be worked on !!!!!!
{
	std::stringstream buffer;
	buffer << "HTTP/1.1 201 Created";
	buffer << CRLFTWO;

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

void Response::_fillTempFile(int i)
{
	this->_receiveMap[i].chunkedTarget = "post_buffer_fd_"/* + i*/;

	std::ofstream buffer;
	buffer.open(this->_receiveMap[i].chunkedTarget);

	if (buffer.is_open() == false)
	{
		throw Response::ERROR_423(); // check if this makes sense, maybe 500 is more appropriate ???????
	}


}

void Response::setPostTarget(int clientFd, std::string target)
{
	int ret = access( target.c_str(), F_OK );
	if (ret == -1)
		this->_receiveMap[clientFd].target = target;
	else
	{
		this->removeFromReceiveMap(clientFd);
		std::cout << "file is already existing" << std::endl; // instead send a response that makes sense
		this->putToResponseMap(this->_createFileExistingHeader(clientFd, target));
	}
}

std::string Response::_createFileExistingHeader(int clientFd, std::string target)
{
	// put stuff here to create a header that makes sense
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

void Response::setPostLength(int clientFd, std::map<std::string, std::string> headerFields)
{
	size_t length = 0;
	if (headerFields.count("Conten-Length"))
	{
		std::cout << headerFields["Content-Length"] << std::endl;
		length = _strToSizeT(headerFields["Content-Length"]);
	}
	else
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

bool Response::checkReceiveExistance(int clientFd)
{
	if (this->_receiveMap.count(clientFd) == 1)
		return (true);
	else
		return (false);
}
