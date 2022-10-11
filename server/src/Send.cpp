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

bool Response::sendRes(int fd)
{
	#ifdef SHOW_LOG_RESPONSE
	std::cout << GREEN << "Sending response to client " << fd << RESET << std::endl;
	#endif
	if (this->_responseMap.count(fd) == 0)
		this->_responseMap[fd].response = this->constructHeader() + this->body;
	int sendSize = MAX_SEND_CHUNK_SIZE;
	if (this->_responseMap[fd].response.size() < MAX_SEND_CHUNK_SIZE)
		sendSize = this->_responseMap[fd].response.size();
	#ifdef SHOW_LOG_RESPONSE
		if (this->_responseMap[fd].response.length() > 2000)
			LOG_RED("Response got truncated!!!");
		LOG_BLUE(this->_responseMap[fd].response.substr(0, 2000)); // BE CAREFULL WITH THIS will print the body of the response, might break your terminal
	#endif
	int n = send(fd, this->_responseMap[fd].response.c_str(), sendSize, 0);
	if (n <= 0)
	{
		std::stringstream errorMessage;
		errorMessage << "send Error sending response for fd: " << fd;
		#ifdef SHOW_LOG
		LOG_RED(errorMessage.str());
		#endif
		std::cerr << RESET;
		return (true);
	}
	#ifdef SHOW_LOG_2
		else
			std::cout << YELLOW << "sent " << n << " bytes to fd: " << fd  << RESET << std::endl;
	#endif
	this->_responseMap[fd].response = this->_responseMap[fd].response.substr(n);
	if (this->_responseMap[fd].response.empty())
	{
		#ifdef SHOW_LOG_2
			std::cout << RED << " FULL Response sent for fd: " << fd << RESET << std::endl;
		#endif
		if (this->_responseMap.count(fd) == 1)
			this->_responseMap.erase(fd);
		return (true);
	}
	return (false);
}
