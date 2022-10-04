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
		std::map<size_t, FILE *> _tempFile;
		std::map<size_t, ResponseStruct> _responseMap; // this will store temp data for sending
		std::string _requestMethod;
		// std::string target;
	// private Methods
		void createMessageMap(void);
		bool isValidStatus(const std::string&);
		void _readForCgi(size_t clientFd);
		// void _createFileExistingHeader(int clientFd, const Request &request, int port);
		size_t _handleChunked(int i);
	public:
		Response(void);
		~Response(void);

		bool isInResponseMap(int clientFd);
		void setIsCgi(int clientFd, bool state);
		bool isFinished(size_t clientFd);
		bool isCgi(size_t clientFd);
		bool isChunked(size_t clientFd);
		FILE *getTempFile(size_t clientFd);
		void removeTempFile(size_t clientFd);
		// std::string lastExitStatus;

		void receiveChunk(int i);
		bool isInReceiveMap(int clientFd); // to verify wether the receiveMap has a key for the filedescriptor
		void constructPostResponse();

		// void setProtocol(const std::string&);
		void setStatus(const std::string&);
		void setBody(const std::string&);
		// void setTarget(const std::string&);
		void setFd(int); // is this used???
		void setProtocol(const std::string&);
		void setRequestMethod(const std::string&);
		void setRequestHead(std::string requestHead, size_t clientFd);
		std::string getRequestHead(size_t clientFd);

		// bool was3XXCode(int clientFd); // for keep-alive

	// for POST requests
		void setPostTarget(int clientFd, std::string target);
		void setPostLength(int clientFd, std::map<std::string, std::string> &headerFields);
		void setPostBufferSize(int clientFd, size_t bufferSize);
		// void checkPostTarget(int clientFd, const Request &request, int port);
		void setPostChunked(int clientFd/* , std::string target */, std::map<std::string, std::string> &headerFields);

		const std::string& getStatus(void) const;
		const std::string& getStatusMessage(void) const;
		const std::map<std::string, std::string>& getMessageMap(void) const;
		std::string getResponse();
		std::string getRequestMethod(void) const;


		std::string constructHeader(void);

		void putToResponseMap(int fd);

		void clear(void);
		void clearResponseMap();
		void removeFromResponseMap(int fd);
		void removeFromReceiveMap(int fd);
		// void init(const Request&);
		// void init(const std::string&, int, const std::string&);
		void addContentLengthHeaderField(void);
		void createBodyFromFile(const std::string&);
		void createIndex(const Request&);
		void createErrorBody(void);
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

	class MissingChunkContentLengthException : public std::exception
	{
		public:
			virtual const char* what() const throw();
	};

	class BadRequestException : public std::exception
	{
		public:
			virtual const char* what() const throw();
	};
};


std::ostream& operator<<(std::ostream&, const Response&);

bool targetExists(const std::string&);

#endif

/********** LEGACY CONTENT **********/

		// int sendall(const int sock_fd, char *buffer, const int len) const; // LEGACY
		// void sendChunk(int i);// i is the fd // LEGACY
		// void endChunkedMessage(int i, int n); // LEGACY
		// bool sendResponse(int); // LEGACY
		// std::string constructChunkedHeader(void); // LEGACY
