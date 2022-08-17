#include "Response.hpp"
#include <string> //std::string
#include <map> //std::map
#include <sstream> //std::stringstream
#include <iostream> //std::ios
#include <fstream> //std::ifstream
#include <sys/socket.h> // send
#include <dirent.h> // dirent, opendir
// #include <sys/types.h>  // opendir
#include <unistd.h>
#include <sys/stat.h> // stat

#include <iomanip> // setfill

//optional one for the server to send the response to the client
bool Response::sendRes(int fd)
{
	std::cout << "In SENDRES Request Method: " << this->_requestMethod << std::endl;

	std::cout << GREEN << "Sending response to client" << RESET << std::endl;
	if (this->_responseMap.count(fd) == 0)
	{
		if (this->_requestMethod != "HEAD")
			this->_responseMap[fd].response = this->constructHeader() + this->body; // only put the body in here
		else
			this->_responseMap[fd].response = this->constructHeader();
	}
	int sendSize = MAX_SEND_CHUNK_SIZE;
	if (this->_responseMap[fd].response.size() < MAX_SEND_CHUNK_SIZE)
		sendSize = this->_responseMap[fd].response.size();
	int n = send(fd, this->_responseMap[fd].response.c_str(), sendSize, 0);
	if (n == -1)
	{
		std::stringstream errorMessage;
		errorMessage << "send Error sending response for fd: " << fd;
		LOG_RED(errorMessage.str());
		// perror(NULL); // check if illegal!!!!!!
		std::cerr << RESET;
		return (true); // throw exception
	}
	#ifdef SHOW_LOG_2
		else
	// disable after testing!!!!!!!!!!
		// std::cout << BLUE << "was send >" << RESET << this->_responseMap[fd].response << BLUE << "<" << RESET << std::endl;
	//
			std::cout << YELLOW << "sent " << n << " bytes to fd: " << fd  << RESET << std::endl;
	#endif
	this->_responseMap[fd].response = this->_responseMap[fd].response.substr(n);
	if (this->_responseMap[fd].response.empty())
	{
		#ifdef SHOW_LOG_2
			std::cout << RED << " FULL Response sent for fd: " << fd << RESET << std::endl;
		#endif
		// close(fd); // never ever close here, we need to rely on the remove client to close the connection, otherwise keep-alive does not work!
		this->lastExitStatus = this->_responseMap[fd].status; // this might be temporary
		if (this->_responseMap.count(fd) == 1)
			this->_responseMap.erase(fd);
		return (true);
	}
	return (false);
}

bool Response::handleClientDisconnect(int fd)
{
	char buf[100];

	while(read(fd, buf, 100)); // this is bad, might lead to hanging connections if a lot of data was put on the fd, but maybe not that bad since a fd can only hold 64kb at once

	close(fd);
	this->_responseMap.erase(fd);
	throw Response::ClientDisconnectException();
}

/********** LEGACY CODE BELOW **********/

// bool Response::sendResponse(int fd)
// {
// 	// return true;
// 	// std::cout << "MAX_SEND_CHUNK_SIZE: " << MAX_SEND_CHUNK_SIZE << std::endl;
// // old start
// 	// std::string response = this->constructHeader() + this->body;
// 	// sendall(fd, (char *)response.c_str(), response.length());
// // old end
// 	if (this->_responseMap.count(fd) == 0)
// 	{
// 		if (this->body.size() > MAX_SEND_CHUNK_SIZE)
// 		{
// 			this->_responseMap[fd].header = this->constructChunkedHeader();
// 			this->_responseMap[fd].response = this->body; // only put the body in here
// 		}
// 		else
// 		{
// 			this->_responseMap[fd].response = this->constructHeader() + this->body; // only put the body in here
// 		}
// 		this->_responseMap[fd].total = this->_responseMap[fd].response.length();
// 		this->_responseMap[fd].bytesLeft = this->_responseMap[fd].response.length();
// 	}
// 	try
// 	{
// 		sendChunk(fd);
// 	}
// 	catch(const std::exception& e)
// 	{
// 		std::cerr << e.what() << '\n';
// 		return (true);
// 	}

// 	if (this->_responseMap.count(fd) == 0)
// 		return (true);
// 	return (false);
// }

// template< typename T >
// static std::string intToHexString(T number)
// {
// 	std::stringstream converted;
// 	converted	/*<< "0x"*/
// 				// << std::setfill ('0') << std::setw(sizeof(T)*2)
// 				<< std::hex << number;
// 	return (converted.str());
// }

// void Response::sendChunk(int i)
// {
// 	const int clientFd = i;
// 	if (this->_responseMap.count(i) == 0)
// 	{
// 		std::cout << RED << "No response found for fd: " << i << RESET << std::endl;
// 		return ;
// 	}
// 	int total = this->_responseMap[i].total;
// 	int bytesLeft = this->_responseMap[i].bytesLeft;

// 	int n = 0;
// 	std::string tempBuffer = this->_responseMap[i].buffer;
// 	int bufferLength = tempBuffer.length();

// 	if (bufferLength > 0)
// 	{
// 		n = send(clientFd, (char *)tempBuffer.c_str(), bufferLength, 0);
// 		// std::cout << BOLD << GREEN << "Bytes send to " << clientFd << ": " << n << RESET << std::endl;
// 		if (n == -1)
// 		{
// 			handleClientDisconnect(clientFd);
// 		}
// 		else if ( n < bufferLength)
// 		{
// 			this->_responseMap[i].buffer = tempBuffer.substr(n);
// 			std::cout << "Map buffer now has a length of: " << this->_responseMap[i].buffer.length() << std::endl;
// 		}
// 	}
// 	else if (this->_responseMap[i].header.length() != 0)
// 	{
// 		tempBuffer = this->_responseMap[i].header;
// 		bufferLength = tempBuffer.length();

// 		n = send(clientFd, (char *)tempBuffer.c_str(), bufferLength, 0);
// 		if (n == -1)
// 		{
// 			handleClientDisconnect(clientFd);
// 		}
// 		else if ( n < bufferLength)
// 		{
// 			this->_responseMap[i].buffer = tempBuffer.substr(n);
// 			std::cout << "Map buffer now has a length of: " << this->_responseMap[i].buffer.length() << std::endl;
// 		}

// 		std::cout	<< YELLOW << "Message send >" << RESET << std::endl
// 					<< this->_responseMap[i].header
// 					<< YELLOW << "<" << RESET << std::endl;
// 		this->_responseMap[i].header.clear();

// 		// std::string test = CRLF;
// 		// std::cout << RED << BOLD << ">" << tempBuffer << "< has a length of" << n << RESET << std::endl;
// 	}
// 	else if (total > MAX_SEND_CHUNK_SIZE && bytesLeft > MAX_SEND_CHUNK_SIZE)
// 	{
// 		tempBuffer = intToHexString(MAX_SEND_CHUNK_SIZE) + CRLF + this->_responseMap[i].response.substr(total - bytesLeft, MAX_SEND_CHUNK_SIZE) + CRLF;
// 		this->_responseMap[i].bytesLeft -= MAX_SEND_CHUNK_SIZE;
// 		bufferLength = tempBuffer.length();
// 		#ifdef SHOW_LOG_2
// 		std::cout << YELLOW << "bufferLength: " << bufferLength << RESET << std::endl;
// 		#endif

// 		n = send(clientFd, (char *)tempBuffer.c_str(), bufferLength, 0);
// 		if (n == -1)
// 		{
// 			handleClientDisconnect(clientFd);
// 		}
// 		else if ( n < bufferLength)
// 		{
// 			this->_responseMap[i].buffer = tempBuffer.substr(n);
// 			std::cout << "Map buffer now has a length of: " << this->_responseMap[i].buffer.length() << std::endl;
// 		}
// 		// std::cout	<< YELLOW << "Message send >" << RESET << std::endl
// 		// 			<< intToHexString(MAX_SEND_CHUNK_SIZE) << CRLF << this->_responseMap[i].response.substr(total - bytesLeft, MAX_SEND_CHUNK_SIZE) << CRLF
// 		// 			<< YELLOW << "<" << RESET << std::endl;
// 	}
// 	else if (total > MAX_SEND_CHUNK_SIZE && bytesLeft < MAX_SEND_CHUNK_SIZE)
// 	{
// 		tempBuffer = intToHexString(bytesLeft) + CRLF + this->_responseMap[i].response.substr(total - bytesLeft) + CRLF;
// 		this->_responseMap[i].bytesLeft -= bytesLeft;
// 		bufferLength = tempBuffer.length();
// 		#ifdef SHOW_LOG_2
// 		std::cout << YELLOW << "last bufferLength: " << bufferLength << RESET << std::endl;
// 		#endif

// 		n = send(clientFd, (char *)tempBuffer.c_str(), bufferLength, 0);
// 		if (n == -1)
// 		{
// 			handleClientDisconnect(clientFd);
// 		}
// 		else if ( n < bufferLength)
// 		{
// 			this->_responseMap[i].buffer = tempBuffer.substr(n);
// 			std::cout << "Map buffer now has a length of: " << this->_responseMap[i].buffer.length() << std::endl;
// 		}

// 		// std::cout	<< YELLOW << "Message send >" << RESET << std::endl
// 		// 			<< (intToHexString(MAX_SEND_CHUNK_SIZE) + CRLF + this->_responseMap[i].response.substr(total - bytesLeft, (bytesLeft + intToHexString(bytesLeft).length())) + CRLF)
// 		// 			<< YELLOW << "<" << RESET << std::endl;
// 	}
// 	else
// 	{
// 		tempBuffer = this->_responseMap[i].response;
// 		this->_responseMap[i].bytesLeft -= bytesLeft;
// 		bufferLength = bytesLeft;

// 		n = send(clientFd, (char *)tempBuffer.c_str(), bufferLength, 0);
// 		if (n == -1)
// 		{
// 			handleClientDisconnect(clientFd);
// 		}
// 		else if ( n < bufferLength)
// 		{
// 			this->_responseMap[i].buffer = tempBuffer.substr(n);
// 			std::cout << "Map buffer now has a length of: " << this->_responseMap[i].buffer.length() << std::endl;
// 		}

// 		// std::cout	<< YELLOW << "Message send >" << RESET << std::endl
// 		// 			<< this->_responseMap[i].response
// 		// 			<< YELLOW << "<" << RESET << std::endl;
// 	}
// 	#ifdef SHOW_LOG_2
// 	std::cout << BOLD << GREEN << "Bytes send to " << clientFd << ": " << n << " (" << this->_responseMap[i].bytesLeft << " bytes remaining)" << RESET << std::endl;
// 	#endif
// 	endChunkedMessage(i, n);
// }

// void Response::endChunkedMessage(int i, int n)
// {
// 	int clientFd = i;
// 	#ifdef SHOW_LOG
// 		if (this->_responseMap[i].buffer.length() > 0)
// 		{
// 			std::cout << BOLD << YELLOW << "Bytes left for " << clientFd << " in buffer: " << this->_responseMap[i].buffer.length() << RESET << std::endl;
// 		}
// 	#endif
// 	if (n == -1)
// 	{
// 		#ifdef SHOW_LOG
// 			perror("send"); //remove
// 		#endif
// 		// this->_responseMap.erase(i);
// 		// throw Response::InternalServerErrorException(); // check if this gets through to send an error to the fd !!!!!!!!!!
// 		// maybe use the create errorHead + errorBody instead here ????
// 			handleClientDisconnect(clientFd);
// 	}
// 	else if (this->_responseMap[i].bytesLeft == 0)
// 	{
// 		if (this->_responseMap[i].total > MAX_SEND_CHUNK_SIZE)
// 		{
// 			std::stringstream bufferStream;
// 			bufferStream << "0" << CRLFTWO;
// 			std::string buffer = bufferStream.str();
// 			int sendReturn = send (clientFd, (char *)buffer.c_str(), 5, 0);
// 			if (sendReturn == -1)
// 			{
// 				handleClientDisconnect(clientFd);
// 			}
// 			else if (sendReturn < 5)
// 			{
// 				std::cout << RED << BOLD << "sending of the final empty chunk failed, only partialy send!!!!!!!" << RESET << std::endl;
// 				// this->_responseMap[i].buffer = buffer.substr(sendReturn);
// 				// return ;
// 			}
// 		}
// 		close(clientFd);
// 		#ifdef SHOW_LOG
// 			std::cout << RED << "fd: " << clientFd << " was closed after sending response" << RESET << std::endl;
// 		#endif
// 		this->_responseMap.erase(i);
// 	}
// }