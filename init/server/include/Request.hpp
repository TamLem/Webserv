#ifndef REQUEST_HPP
#define REQUEST_HPP
#pragma once

#include <string> //std::string
#include <iostream> //std::ostream
#include <set> //std::set
#include <vector> //std::vector
#include "Message.hpp"

#define DEFAULT_INDEX "index.html"
#define BAD_PATH "./pages/404.html"
#define INTERNAL_PATH "./pages/500.html"

class Request : public Message
{
	private:
		std::set<std::string> validMethods;
		std::string method;
		std::string url; // AE remove
		std::string uri;
		std::string query;
		std::string fragment;
		void parseMessage(const std::string&);
		void parseStartLine(std::istringstream&);
		void createStartLineTokens(std::vector<std::string>&, const std::string&) const;
		void breakUpUri(const std::string&);
		bool isValidMethod(const std::string) const;
		void parseHeaderFields(std::istringstream&);
		void parseHeaderFieldLine(const std::string&);
		void createHeaderTokens(std::vector<std::string>& tokens, const std::string& message);
		const std::string createHeaderFieldName(const std::string& message, const size_t pos) const;
		bool isValidHeaderFieldName(const std::string&) const;
		void toLower(std::string&) const;
		const std::string createHeaderFieldValue(const std::string& message, const size_t pos);
		const std::string removeLeadingAndTrailingWhilespaces(const std::string& message, size_t pos) const;
		bool isValidHeaderFieldValue(const std::string&) const;
		void setBodyFlag(void);
		void parseBody(std::istringstream&);
		void addMethods(void);
	public:
		// Request(const std::string&);
		Request(const std::string&);
		// Request(const std::string&, int fd);
		~Request(void);

		// void setMethod(const std::string&);
		void setUri(const std::string&);

		const std::string& getMethod(void) const;
		const std::string& getUrl(void) const;
		const std::string& getUri(void) const;
		const std::string& getQuery(void) const;
		const std::string& getFragment(void) const;
		// const std::string& getProtocol(void) const;

	class InvalidNumberOfTokens : public Message::BadRequest
	{
		const char* what() const throw();
	};

	class EmptyMessage : public Message::BadRequest
	{
		const char* what() const throw();
	};

	class InvalidMethod : public std::exception
	{
		const char* what() const throw();
	};

	class InvalidHeaderField : public Message::BadRequest
	{
		const char* what() const throw();
	};

	class NoHost : public Message::BadRequest
	{
		const char* what() const throw();
	};

	class InvalidHeaderFieldName : public Message::BadRequest
	{
		const char* what() const throw();
	};

	class InvalidHeaderFieldValue : public Message::BadRequest
	{
		const char* what() const throw();
	};

	class InvalidProtocol : public std::exception
	{
		const char* what() const throw();
	};
};

std::ostream& operator<<(std::ostream&, const Request&);

#endif