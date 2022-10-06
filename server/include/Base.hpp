#ifndef BASE_HPP
#define BASE_HPP

#include <map>
#include <set>
#include <string>
#include <iostream>
#include <stdbool.h>
#include <istream>
#include <ios>
#include <netinet/in.h>

/* put here includes and defines that are needed for the whole project only */
// Colors and Printing
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RED "\033[31m"
#define BOLD "\033[1m"
#define UNDERLINED "\033[4m"

#define LOG_RED(x) (std::cout << __FILE__ << ":" << __LINE__ << "\t\033[1;31m" << x << "\033[0m" << std::endl)
#define LOG_YELLOW(x) (std::cout << __FILE__ << ":" << __LINE__ << "\t\033[1;33m" << x << "\033[0m" << std::endl)
#define LOG_GREEN(x) (std::cout << __FILE__ << ":" << __LINE__ << "\t\033[1;32m" << x << "\033[0m" << std::endl)
#define LOG_BLUE(x) (std::cout << __FILE__ << ":" << __LINE__ << "\t\033[1;34m" << x << "\033[0m" << std::endl)

// other defines
#define CRLF "\r\n"
#define CRLFTWO "\r\n\r\n"
#define WHITESPACE "\n\r\t\f\v "
#define DECIMAL "0123456789"
#define SP ' '
#define CR '\r'
#define TCHAR "!#$%&'*+-.^_`|~"

// defines for reading from client
#define MAX_REQUEST_LINE_SIZE 512 // change this to increase the max length of accepted URI
#define MAX_REQUEST_HEADER_SIZE 1024 // change this to increase the size of accepted request-headers
#define MAX_EVENTS 128
#define MAX_SEND_CHUNK_SIZE (1024 * 1024) // this controlls the size of the chunks we are sending back to the client
#define CLIENT_TIMEOUT 60 // this will kick clients after X amount of inactivity
#define DEFAULT_CONFIG "server/config/test.conf" // this is the default config files path if no config file was provided on startup

// ClientStruct
struct ClientStruct
{
	int fd;
	struct sockaddr_in addr;
	time_t timeout;
};

// ResponseStruct
struct ResponseStruct
{
// general
	std::string response;
	size_t total;
	size_t bytesLeft;
// header information
	// std::string protocoll;
	// std::string status;
	// std::string statusMessage;
	std::string target; // this is the target after directory/file routing
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
// identifiers
	bool isChunked;
	bool isCgi;
	bool end;
// general
	std::string requestHead;
	std::string target;
	size_t total;
	size_t bytesWritten;
	size_t bytesLeft;
	size_t bufferSize;
	size_t maxBodySize;
};

// ConfigStruct
struct ConfigStruct
{
// bare minimum of the .conf file
	std::string								serverName;
	std::map<std::string, unsigned short>	listen;
	std::string								root;

// not sure about these two???????????
	std::map<std::string, std::string>		cgi;
	std::string								cgiBin; // is this still needed???????????

// these variables have default values if not set in the .conf file
	size_t									clientBodyBufferSize;
	size_t									clientMaxBodySize;
	std::string								indexPage;
	std::map<std::string, LocationStruct>	location;
	std::map<std::string, std::string>		errorPage;
	bool									autoIndex;
};

#endif // BASE_HPP

// LEGACY from ResponseStruct
	// std::string buffer; // LEGACY for when less than sendChunk() only sends part of the chunk
	// std::string header; // LEGACY for chunked response only, was used in sendResponse()->sendChunk() not sure if still used????
