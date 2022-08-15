#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#pragma once

#include "Base.hpp"
#include "Request.hpp"
#include "Message.hpp"

#include <iostream> //std::ostream
#include <string> //std::string
#include <map> //std::map
#include <sstream> //std::stringstream
#include <fstream> //std::ifstream std::ofstream
#include <sys/socket.h> // send
#include <dirent.h> // dirent, opendir
// #include <sys/types.h>  // opendir
#include <unistd.h> // access
#include <sys/stat.h> // stat

class Response : public Message
{
	private:
	// defines only to not have undefined behaviour
		Response(const Response&);
		Response& operator=(const Response&);

	// private Members
		std::string status;
		std::string statusMessage;
		std::map<std::string, std::string> messageMap;
		std::map<size_t, ReceiveStruct> _receiveMap; // this will store temp data for the POST events
		std::map<size_t, ResponseStruct> _responseMap; // this will store temp data for sending
		// std::string target;
	// private Methods
		void createMessageMap(void);
		bool isValidStatus(const std::string&);
	protected: // why protected??? is there any class inheriting from response???
		int sendall(const int sock_fd, char *buffer, const int len) const;
		void sendChunk(int i);// i is the fd
	public:
		Response(void);
		~Response(void);

		void receiveChunk(int i);
		bool isInReceiveMap(int clientFd);
		std::string constructPostResponse();
		void _fillTempFile(int i);

		// void setProtocol(const std::string&);
		void setStatus(const std::string&);
		void setBody(const std::string&);
		// void setTarget(const std::string&);
		void setFd(int); // is this used???
		void setProtocol(const std::string&);
		void setPostTarget(int clientFd, std::string target);
		void setPostLength(int clientFd, std::map<std::string, std::string> headerFields);
		void setPostBufferSize(int clientFd, size_t bufferSize);
		bool checkReceiveExistance(int clientFd);

		// const std::string& getProtocol(void) const;
		const std::string& getStatus(void) const;
		const std::string& getStatusMessage(void) const;
		const std::map<std::string, std::string>& getMessageMap(void) const;
		std::string getResponse();

		std::string constructHeader(void);
		std::string constructChunkedHeader(void);
		void endChunkedMessage(int i, int n);

		void putToResponseMap(int fd);

		void clear(void);
		void clearResponseMap();
		void removeFromResponseMap(int fd);
		void removeFromReceiveMap(int fd);
		// void init(const Request&);
		// void init(const std::string&, int, const std::string&);
		void addDefaultHeaderFields(void);
		void createBodyFromFile(const std::string&);
		void createIndex(const Request&);
		void createErrorBody(void);
		bool sendResponse(int);
		bool sendRes(int);

	bool handleClientDisconnect(int fd);

	class InvalidStatus : public std::exception
	{
		const char* what() const throw();
	};

	class ERROR_500 : public std::exception
	{
		const char* what() const throw();
	};

	class ERROR_404 : public Message::BadRequest
	{
		const char* what() const throw();
	};

	class ERROR_403 : public Message::BadRequest
	{
		const char* what() const throw();
	};

	class ERROR_423 : public Message::BadRequest
	{
		const char* what() const throw();
	};

	class InvalidProtocol : public std::exception
	{
		const char* what() const throw();
	};

	class InternalServerErrorException : public std::exception
	{
		const char* what() const throw();
	};

	class ClientDisconnectException : public std::exception
	{
		const char* what() const throw();
	};

	class SizeTOverflowException : public std::exception
	{
		public:
			virtual const char* what() const throw();
	};

	class NegativeDecimalsNotAllowedException : public std::exception
	{
		public:
			virtual const char* what() const throw();
	};

	class ClientDisconnect : public std::exception
	{
		public:
			virtual const char* what() const throw();
	};
};

std::ostream& operator<<(std::ostream&, const Response&);

bool targetExists(const std::string&);

#endif