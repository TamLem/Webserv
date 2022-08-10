#ifndef BASE_HPP
#define BASE_HPP

#include <map>
#include <set>
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
#define SP ' '
#define CR '\r'
#define TCHAR "!#$%&'*+-.^_`|~"

// defines for reading from client
#define MAX_REQUEST_HEADER_SIZE 1024
#define MAX_EVENTS 128
#define MAX_REQUEST_LINE_SIZE 512
#define MAX_SEND_CHUNK_SIZE (4 * 1024)

// ResponseStruct
struct ResponseStruct
{
	std::string buffer;
	std::string header;
	std::string response;
	// std::string status;
	// std::string statusMessage;
	size_t total;
	size_t bytesLeft;
};

// LocationStruct
struct LocationStruct
{
	bool isDir;
	bool autoIndex;
	std::set<std::string> allowedMethods;
	std::string root;
	std::string indexPage;
};

// ReceiveStruct
struct ReceiveStruct
{
	std::string target;
	// std::string status;
	// std::string statusMessage;
	size_t total;
	size_t bytesLeft;
	int bufferSize;
};

// ConfigStruct
struct ConfigStruct
{
// bare minimum of the .conf file
	std::string								serverName;
	std::map<std::string, unsigned short>	listen;
	std::string								root;
	std::map<std::string, std::string>		cgi;
	std::string								cgiBin;
	size_t									clientBodyBufferSize;

// these variables have default values if not set in the .conf file
	size_t									clientMaxBodySize;
	std::string								indexPage;
	std::map<std::string, LocationStruct>	location;
	std::map<std::string, std::string>		errorPage;
	bool									autoIndex;
};

#endif // BASE_HPP