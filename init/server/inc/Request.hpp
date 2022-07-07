#pragma once

#include <string>

class Request
{
	private:
		std::string method;
		std::string url;
		std::string protocol;
		//some container fo headers
	public:
		Request(void);
		Request(const Request& other);
		Request& operator=(const Request& other);
		~Request(void);
		std::string construct_header(void);
};