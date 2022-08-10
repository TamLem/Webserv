#include "Response.hpp"

void Response::receiveChunk(int i)
{
	const int clientFd = i;
	size_t total = this->_receiveMap[i].total;
	size_t bytesLeft = this->_receiveMap[i].bytesLeft;
	size_t bufferSize = this->_receiveMap[i].bufferSize;

	int n = 0;
	std::ofstream buffer;
	char chunk[bufferSize];
	// memset(chunk, 0, bufferSize);

	buffer.open(this->_receiveMap[i].target.c_str(), std::ios_base::app);
	// int bufferLength;
	if (total > bufferSize && bytesLeft > bufferSize)
	{
		std::cout << YELLOW << "bytesLeft: " << bytesLeft << RESET << std::endl;

		n = read(clientFd, chunk, bufferSize);

		// std::cout	<< YELLOW << "Message send >" << RESET << std::endl
		// 			<< intToHexString(this->_receiveMap[i].bufferSize) << CRLF << this->_receiveMap[i].response.substr(total - bytesLeft, this->_receiveMap[i].bufferSize) << CRLF
		// 			<< YELLOW << "<" << RESET << std::endl;
	}
	else /*if (total > this->_receiveMap[i].bufferSize && bytesLeft < this->_receiveMap[i].bufferSize)*/ // check if it works like that
	{
		std::cout << YELLOW << "last bytesLeft: " << bytesLeft << RESET << std::endl;
		n = read(clientFd, chunk, bytesLeft);

		// std::cout	<< YELLOW << "Message send >" << RESET << std::endl
		// 			<< (intToHexString(this->_receiveMap[i].bufferSize) + CRLF + this->_receiveMap[i].response.substr(total - bytesLeft, (bytesLeft + intToHexString(bytesLeft).length())) + CRLF)
		// 			<< YELLOW << "<" << RESET << std::endl;
	}
	std::cout << BOLD << GREEN << "Bytes received from " << clientFd << ": " << n << RESET << std::endl;
	if (n > 0)
	{
		// if (n > bytesLeft)
		// {
		// 	std::cout << "CHECK n FOR RECEIVED BYTES!!!!!!!" << std::endl;
		// 	bytesLeft = 0;
		// }
		// else
			bytesLeft -= n;
		std::cout << BOLD << YELLOW << "Bytes left to read from " << clientFd << ": " << bytesLeft << RESET << std::endl;
	}
	else if (n == -1)
	{
		perror("send");
		this->_receiveMap.erase(i);
		bytesLeft = 0;
		buffer.close();
		throw Response::InternalServerErrorException();
	}
	buffer << chunk;
	if (bytesLeft)
	{
		this->_receiveMap[i].bytesLeft = bytesLeft;
		buffer.close();
	}
	else
	{
		buffer.close();
		// send response here!!!!!!!!!!!!!
		// close(clientFd);
		#ifdef SHOW_LOG
			std::cout << RED << "fd: " << clientFd << " was closed after sending response" << RESET << std::endl;
		#endif
		this->_receiveMap.erase(i);
		this->_responseMap[clientFd].response = this->constructPostResponse();
	}
}

std::string Response::constructPostResponse()
{
	std::stringstream buffer;
	buffer << "201 Created";

	return (buffer.str());
}

// std::string Response::constructFailedPostResponse()
// {
// 	std::stringstream buffer;
// 	buffer << "500";

// 	return (buffer.str());
// }

bool Response::isInReceiveMap(int clientFd)
{
	return (this->_receiveMap.count(clientFd));
}
