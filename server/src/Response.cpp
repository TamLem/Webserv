#include "Response.hpp"

#include <string> //std::string
#include <map> //std::map
#include <sstream> //std::stringstream
#include <iostream> //std::ios
#include <fstream> //std::ifstream
#include <sys/socket.h> // send
#include <dirent.h> // dirent, opendir
// #include <sys/types.h>  // opendir
#include <unistd.h>
#include <sys/stat.h> // stat

#include <iomanip> // setfill

#define RESET "\033[0m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RED "\033[31m"

bool Response::isValidStatus(const std::string& status)
{
	if (this->messageMap.count(status))
		return (true);
	return (false);
}

Response::Response(void)
{
	#ifdef SHOW_CONSTRUCTION
		std::cout << GREEN << "Response Default Constructor called for " << this << RESET << std::endl;
	#endif
	this->createMessageMap();
}

void Response::clear(void)
{
	this->protocol = "";
	this->body = "";
	hasBody = false;
	this->headerFields.clear();
	this->status = "";
	this->statusMessage = "";
	target = "";
}

Response::~Response(void)
{
	#ifdef SHOW_CONSTRUCTION
		std::cout << RED << "Response Deconstructor called for " << this << RESET << std::endl;
	#endif
}

void Response::setStatus(const std::string& status)
{
	if (!isValidStatus(status))
		throw InvalidStatus();
	this->status = status;
	this->statusMessage = this->messageMap[this->status];
}

void Response::setBody(const std::string& body)
{
	this->body = body;
}

// void Response::setTarget(const std::string& target)
// {
// 	this->target = target;
// }

void Response::setProtocol(const std::string& protocol)
{
	if (!isValidProtocol(protocol))
		throw Response::InvalidProtocol();
	this->protocol = protocol;
}

const std::string& Response::getStatus(void) const
{
	return (this->status);
}

const std::string& Response::getStatusMessage(void) const
{
	return (this->statusMessage);
}

const std::map<std::string, std::string>& Response::getMessageMap(void) const
{
	return (this->messageMap);
}

std::string Response::constructHeader(void)
{
	std::stringstream stream;

	stream << this->protocol << " " << this->status << " " << this->statusMessage << CRLF;
	for (std::map<std::string, std::string>::const_iterator it = this->headerFields.begin(); it != this->headerFields.end(); ++it)
	{
		stream << it->first << ": "
		<< it->second << CRLF;
	}
	stream << CRLF;
	return (stream.str());
}

std::string Response::constructChunkedHeader(void)
{
	std::stringstream stream;

	stream << this->protocol << " " << this->status << " " << this->statusMessage << CRLF;
	// stream << "Content-Type: " << "image/jpg" << CRLF;
	stream << "Transfer-Encoding: chunked" << CRLFTWO;

	return (stream.str());
}

int Response::sendall(const int sock_fd, char *buffer, const int len) const
{
	int total;
	int bytesleft;
	int n;

	total = len;
	bytesleft = len;
	while (total > 0)
	{
		n = send(sock_fd, buffer, bytesleft, 0);
		if (n == -1)
		{
			perror("send");
			return (-1);
		}
		total -= n;
		bytesleft -= n;
		buffer += n;
	}
	close(sock_fd);
	#ifdef SHOW_LOG
		std::cout << RED << "fd: " << sock_fd << " was closed after sending response" << RESET << std::endl;
	#endif
	return (0);
}

void Response::createErrorBody(void)
{
	std::stringstream body;
	body <<
	"<html>\n\
	<head>\n\
	<title>Error " << this->status << "</title>\n\
	<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"images/favicon.ico\">\n\
	</head>\n\
	<body bgcolor=\"000000\">\n\
	<center>\n\
	<h1 style=\"color:white\">Error " << this->status << "</h1>\n\
	<p style=\"color:white\">" << this->statusMessage << "!\n\
	<br><br>\n\
	<img src=\"/images/error.jpg\" align=\"TOP\">\n\
	</center>\n\
	</body>\n\
	</html>";
	this->body = body.str();
}

// static std::string staticReplaceInString(std::string str, std::string tofind, std::string toreplace)
// {
// 		size_t position = 0;
// 		for ( position = str.find(tofind); position != std::string::npos; position = str.find(tofind,position) )
// 		{
// 				str.replace(position , tofind.length(), toreplace);
// 		}
// 		return(str);
// }

void Response::createIndex(const std::string& path)
{
	std::stringstream body;
	body <<
	"<html>\n\
	<head>\n\
	<title>Index</title>\n\
	<meta charset=\"UTF-8\">\n\
	<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"images/favicon.ico\">\n\
	</head>\n\
	<body bgcolor=\"FFFFFF\">\n\
	<center>\n\
	<h1 style=\"color:black\">Index of/\n\
	</h1>\n\
	</center>\n\
	<div style=\"margin-left:0%\">\n\
	<ul>";
	DIR *d;
	struct dirent *dir;
	std::string name;
	// d = opendir(target.substr(0, target.find_last_of('/')).c_str()); // AE better keep path and file seperate
	d = opendir(path.c_str()); // AE better keep path and file seperate
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			name = dir->d_name;
			// name = staticReplaceInString(name, "u%CC%88", "ü");
			// name = staticReplaceInString(name, "a%CC%88", "ä");
			// name = staticReplaceInString(name, "o%CC%88", "ö");
			// if (dir->d_type == DT_REG) //only files
			// {
				// std::cerr << BOLD << RED << "dir: " << name << RESET << std::endl;
				// if (strcmp(name, "..") == 0)
				if (name.compare("..") == 0)
				{
					body << "<li><a href=\"" << name << "\">" << "Parent Directory" << "</a></li>\n";
				}
				// else if (strlen(name) != 0 && strcmp(name, ".") != 0)
				else if (name.length() != 0 && name.compare(".") != 0)
				{
					if (dir->d_type == DT_DIR)
						body << "<li><a href=\"" << name << "/\">" << name << "</a></li>\n";
					else
						body << "<li><a href=\"" << name << "\">" << name << "</a></li>\n";
				}

				// body << "<li style=\"color:blue\">" << name << "<li/>";
			// }
		}
		body <<
		"</div>\n\
		</body>\n\
		</html>";
		this->body = body.str();
		closedir(d);
	}
	else
	{
		perror(NULL);
		throw ERROR_404(); // AE exception
		//AE 500 response
	}
}

static bool staticTargetIsDir(const std::string& target)
{
	struct stat statStruct;

	// error will return false
	if (stat(target.c_str(), &statStruct) == 0)
	{
		if (statStruct.st_mode & S_IFDIR)
			return (true);
	}
	return (false);
}

void Response::createBodyFromFile(const std::string& target)
{
	std::stringstream tempBody;
	std::ifstream file(target.c_str(), std::ios::binary);
	// std::cerr << BOLD << RED << "target:" << target << RESET << std::endl;
	if (staticTargetIsDir(target))
		throw ERROR_404();
	if (file.is_open())
	{
		// std::cerr << BOLD << RED << "open" << RESET << std::endl;
		tempBody << file.rdbuf();
		file.close();
		this->body = tempBody.str();
	}
	else
	{
		perror(NULL);
		throw ERROR_404();
		//404 response
	}
}

void Response::addDefaultHeaderFields(void)
{
	std::stringstream contentLength;
	// addHeaderField("Server", "localhost:8080");
	if (headerFields.count("Transfer-Encoding") == 0)
	{
		contentLength << this->body.length();
		addHeaderField("Content-Length", contentLength.str());
	}
}

bool Response::sendResponse(int fd)
{
// old start
	// std::string response = this->constructHeader() + this->body;
	// sendall(fd, (char *)response.c_str(), response.length());
// old end
	if (this->_responseMap.count(fd) == 0)
	{
		if (this->body.size() > MAX_SEND_CHUNK_SIZE)
		{
			this->_responseMap[fd].header = this->constructChunkedHeader();
			this->_responseMap[fd].response = this->body; // only put the body in here
		}
		else
		{
			this->_responseMap[fd].response = this->constructHeader() + this->body; // only put the body in here
		}
		this->_responseMap[fd].total = this->_responseMap[fd].response.length();
		this->_responseMap[fd].bytesLeft = this->_responseMap[fd].response.length();
	}
	sendChunk(fd);
	if (this->_responseMap.count(fd) == 0)
		return (true);
	return (false);
}

void Response::createMessageMap(void)
{
	//1xx informational response
	this->messageMap["100"] = "Continue";
	this->messageMap["101"] = "Switching Protocols";
	this->messageMap["102"] = "Processing";
	this->messageMap["103"] = "Early Hints";
	//2xx success
	this->messageMap["200"] = "OK";
	this->messageMap["201"] = "Created";
	this->messageMap["202"] = "Accepted";
	this->messageMap["203"] = "Non-Authoritative Information";
	this->messageMap["204"] = "No Content";
	this->messageMap["205"] = "Reset Content";
	this->messageMap["206"] = "Partial Content";
	this->messageMap["207"] = "Multi-Status";
	this->messageMap["208"] = "Already Reported";
	this->messageMap["226"] = "IM Used";
	//3xx redirection
	this->messageMap["300"] = "Multiple Choices";
	this->messageMap["301"] = "Moved Permanently";
	this->messageMap["302"] = "Found";
	this->messageMap["303"] = "See Other";
	this->messageMap["304"] = "Not Modified";
	this->messageMap["305"] = "Use Proxy";
	this->messageMap["306"] = "Switch Proxy";
	this->messageMap["307"] = "Temporary Redirect";
	this->messageMap["308"] = "Permanent Redirect";
	//4xx client errors
	this->messageMap["400"] = "Bad Request";
	this->messageMap["401"] = "Unauthorized";
	this->messageMap["402"] = "Payment Required";
	this->messageMap["403"] = "Forbidden";
	this->messageMap["404"] = "Not Found";
	this->messageMap["405"] = "Method Not Allowed";
	this->messageMap["406"] = "Not Acceptable";
	this->messageMap["407"] = "Proxy Authentication Required";
	this->messageMap["408"] = "Request Timeout";
	this->messageMap["409"] = "Conflict";
	this->messageMap["410"] = "Gone";
	this->messageMap["411"] = "Length Required";
	this->messageMap["412"] = "Precondition Failed";
	this->messageMap["413"] = "Content Too Large";
	this->messageMap["414"] = "URI Too Long";
	this->messageMap["415"] = "Unsupported Media Type";
	this->messageMap["416"] = "Range Not Satisfiable";
	this->messageMap["417"] = "Expectation Failed";
	this->messageMap["418"] = "I'm a teapot";
	this->messageMap["421"] = "Misdirected Request";
	this->messageMap["422"] = "Unprocessable Content";
	this->messageMap["423"] = "Locked";
	this->messageMap["424"] = "Failed Dependency";
	this->messageMap["425"] = "Too Early";
	this->messageMap["426"] = "Upgrade Required";
	this->messageMap["428"] = "Precondition Required";
	this->messageMap["429"] = "Too Many Requests";
	this->messageMap["431"] = "Request Header Fields Too Large";
	this->messageMap["451"] = "Unavailable For Legal Reasons";
	//5xx server errors
	this->messageMap["500"] = "Internal Server Error";
	this->messageMap["501"] = "Not Implemented";
	this->messageMap["502"] = "Bad Gateway";
	this->messageMap["503"] = "Service Unavailable";
	this->messageMap["504"] = "Gateway Timeout";
	this->messageMap["505"] = "HTTP Version Not Supported";
	this->messageMap["506"] = "Variant Also Negotiates";
	this->messageMap["507"] = "Insufficient Storage";
	this->messageMap["508"] = "Loop Detected";
	this->messageMap["510"] = "Not Extended";
	this->messageMap["511"] = "Network Authentication Required";
}

std::ostream& operator<<(std::ostream& out, const Response& response)
{
	out << response.getProtocol() << " "
	<< response.getStatus() << " "
	<< response.getStatusMessage() << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = response.getHeaderFields().begin(); it != response.getHeaderFields().end(); ++it)
	{
		out << it->first << ": "
		<< it->second << "\n";
	}
	out << response.getBody() << std::endl;
	return (out);
}

const char* Response::InvalidStatus::what() const throw()
{
	return ("400");
}

const char* Response::ERROR_404::what() const throw()
{
	return ("404");
}

const char* Response::InvalidProtocol::what() const throw() //AE is it good to have different codes for request/response?
{
	return ("500");
}

const char* Response::InternalServerErrorException::what() const throw()
{
	return ("500");
}
