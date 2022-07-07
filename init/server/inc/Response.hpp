#pragma once

#include <string>
#include <map>

class Response
{
	private:
		std::string protocol;
		int status;
		std::map<int, std::string> messages;
		//container of header fields
		Response(const Response& other);
		Response& operator=(const Response& other);
		void create_messages(void);
	public:
		Response(void);
		// Response(std::map<int, std::string>*);
		~Response(void);

		void setProtocol(const std::string&);
		void setStatus(const int&);

		const std::string& getProtocol(void) const;
		const int& getStatus(void) const;

		std::string construct_header(void);
};