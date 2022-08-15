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



/********** handle chunked POST **********/
void Response::_fillTempFile(int i)
{
	this->_receiveMap[i].chunkedTarget = "post_buffer_fd_" + i;

	std::ofstream buffer;
	buffer.open(this->_receiveMap[i].chunkedTarget);

	if (buffer.is_open() == false)
	{
		throw Response::ERROR_423(); // check if this makes sense, maybe 500 is more appropriate ???????
	}


}
