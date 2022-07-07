#pragma once

#include <string>

class Request
{
	private:
		std::string method;
		std::string url;
		std::string protocol;
		//some container fo headers
		Request(const Request&);
		Request& operator=(const Request&);
	public:
		Request(void);
		~Request(void);

		void setMethod(const std::string&);
		void setUrl(const std::string&);
		void setProtocol(const std::string&);

		const std::string& getMethod(void) const;
		const std::string& getUrl(void) const;
		const std::string& getProtocol(void) const;

		// std::string construct_header(void);
};

std::ostream& operator<<(std::ostream&, const Request&);