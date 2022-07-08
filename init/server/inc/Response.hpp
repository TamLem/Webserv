#pragma once

#include <string>
#include <map>
#include <iostream> //std::ostream

class Response
{
	private:
		std::string protocol;
		int status;
		std::string message;
		std::map<int, std::string> messages;
		//container of header fields
		Response(void);
		Response(const Response&);
		Response& operator=(const Response&);
		void create_messages(void);
		bool is_valid_protocol(std::string);
		bool is_valid_status(int);
	public:
		Response(std::string, int);
		// Response(std::map<int, std::string>*);
		~Response(void);

		// void setProtocol(const std::string&);
		// void setStatus(const int&);

		const std::string& getProtocol(void) const;
		const int& getStatus(void) const;
		const std::string& getMessage(void) const;

		std::string construct_header(void);

	class InvalidProtocol : public std::exception
	{
		const char* what() const throw();
	};

	class InvalidStatus : public std::exception
	{
		const char* what() const throw();
	};
};

std::ostream& operator<<(std::ostream&, const Response&);