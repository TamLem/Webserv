#pragma once

#include <string>
#include <iostream> //std::ostream

class Request
{
	private:
		std::string method;
		std::string url;
		std::string protocol;
		//some container fo headers
		Request(const Request&);
		Request& operator=(const Request&);
		Request(void);
		void parseMessage(const std::string& message);
	public:
		Request(const std::string&);
		~Request(void);

		void setMethod(const std::string&);
		void setUrl(const std::string&);
		void setProtocol(const std::string&);

		const std::string& getMethod(void) const;
		const std::string& getUrl(void) const;
		const std::string& getProtocol(void) const;

		// std::string construct_header(void);
	class InvalidInput : public std::exception
	{
		const char* what() const throw();
	};
};

std::ostream& operator<<(std::ostream&, const Request&);