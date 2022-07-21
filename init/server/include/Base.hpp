#ifndef BASE_HPP
#define BASE_HPP

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <stdbool.h>

/* put here includes and defines that are needed for the whole project only */
// Colors and Printing
#ifndef RESET
	#define RESET "\033[0m"
#endif

#ifndef GREEN
	#define GREEN "\033[32m"
#endif

#ifndef YELLOW
	#define YELLOW "\033[33m"
#endif

#ifndef BLUE
	#define BLUE "\033[34m"
#endif

#ifndef RED
	#define RED "\033[31m"
#endif

#ifndef BOLD
	#define BOLD "\033[1m"
#endif

#ifndef UNDERLINED
	#define UNDERLINED "\033[4m"
#endif


// other defines
#ifndef CRLF
	#define CRLF "\r\n"
#endif

#ifndef CRLFTWO
	#define CRLFTWO "\r\n\r\n"
#endif

#ifndef WHITESPACE
	#define WHITESPACE "\n\r\t\f\v "
#endif

#ifndef DECIMAL
	#define DECIMAL "0123456789"
#endif

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
	bool									autoIndex;
	std::string								indexPage;
	bool									chunkedTransfer;
	size_t									clientBodyBufferSize;
	size_t									clientMaxBodySize;
	std::vector<std::string>				cgi;
	std::string								cgiBin;
	std::map<std::string, LocationStruct>	location;
	std::vector<std::string>				errorPage;
	bool									showLog;
};


#endif // BASE_HPP