#ifndef BASE_HPP
#define BASE_HPP

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <stdbool.h>

/* put here includes and defines that are needed for the whole project only */
// Colors and Printing
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RED "\033[31m"
#define BOLD "\033[1m"
#define UNDERLINED "\033[4m"

// other defines
#define CRLF "\r\n"
#define CRLFTWO "\r\n\r\n"
#define WHITESPACE "\n\r\t\f\v "
#define DECIMAL "0123456789"

// LocationStruct
struct LocationStruct
{
	bool isDir;
	std::string root;
	std::string indexPage;
	bool autoIndex;
	bool getAllowed;
	bool postAllowed;
	bool deleteAllowed;
};

// ConfigStruct
struct ConfigStruct
{
	std::string								serverName;
	std::vector<std::string>				listen;
	std::string								root;
	std::vector<std::string>				cgi;
	std::string								cgiBin;
	size_t									clientBodyBufferSize;

	size_t									clientMaxBodySize;
	std::string								indexPage;
	std::map<std::string, LocationStruct>	location;
	std::vector<std::string>				errorPage;
	bool									autoIndex;
	bool									showLog;
	bool									chunkedTransfer;
};


#endif // BASE_HPP