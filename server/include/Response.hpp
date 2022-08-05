#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#pragma once

#include <string>
#include <map>
#include <iostream> //std::ostream
#include "Message.hpp"
#include "Request.hpp"

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
		std::map<size_t, ResponseStruct> _responseMap; // this will store all the data for sending
		// std::string target;
	// private Methods
		void createMessageMap(void);
		bool isValidStatus(const std::string&);
	protected:
		int sendall(const int sock_fd, char *buffer, const int len) const;
		void sendChunk(int i);// i is the fd
	public:
		Response(void);
		~Response(void);

		// void setProtocol(const std::string&);
		void setStatus(const std::string&);
		void setBody(const std::string&);
		// void setTarget(const std::string&);
		void setFd(int); // is this used???
		void setProtocol(const std::string&);

		// const std::string& getProtocol(void) const;
		const std::string& getStatus(void) const;
		const std::string& getStatusMessage(void) const;
		const std::map<std::string, std::string>& getMessageMap(void) const;

		std::string constructHeader(void);
		std::string constructChunkedHeader(void);

		void clear(void);
		// void init(const Request&);
		// void init(const std::string&, int, const std::string&);
		void addDefaultHeaderFields(void);
		void createBodyFromFile(const std::string&);
		void createIndex(const std::string&);
		void createErrorBody(void);
		bool sendResponse(int);

	class InvalidStatus : public std::exception
	{
		const char* what() const throw();
	};

	class ERROR_404 : public Message::BadRequest
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
};

std::ostream& operator<<(std::ostream&, const Response&);

#endif