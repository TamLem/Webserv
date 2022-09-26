#include "Response.hpp"
#include "Utils.hpp"

//// might be temporary
	bool Response::isInResponseMap(int clientFd)
	{
		return (this->_responseMap.count(clientFd));
	}
////
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

void Response::setRequestMethod(const std::string& method)
{
	this->_requestMethod = method;
}

std::string Response::getRequestMethod(void) const
{
	return this->_requestMethod;
}


void Response::clear(void)
{
	this->protocol = "";
	this->body = "";
	hasBody = false;
	this->headerFields.clear();
	this->status = "";
	this->statusMessage = "";
	this->_requestMethod = "";
	target = "";
}

void Response::clearResponseMap()
{
	this->_responseMap.clear();
}

void Response::removeFromResponseMap(int fd)
{
	if (this->_responseMap.count(fd) == true)
		this->_responseMap.erase(fd);
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

void Response::setProtocol(const std::string& protocol)
{
	if (!isValidProtocol(protocol))
		throw Response::InvalidProtocol();
	this->protocol = protocol;
}

bool Response::was3XXCode(int clientFd)
{
	(void)clientFd;
	// if (this->_responseMap.count(clientFd))
		// return (this->_responseMap[clientFd].status[0] == '3');
		return (this->status[0] == '3');
	// else
	// 	return (false);
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

std::string Response::getResponse()
{
	#ifdef SHOW_LOG_2
	std::stringstream message;
	message << "Request Method: " << this->_requestMethod;
	LOG_YELLOW(message.str());
	#endif
	std::stringstream buffer;
	buffer << this->constructHeader();
	#ifdef FORTYTWO_TESTER
	if (this->_requestMethod != "HEAD")
	#endif
		buffer << this->getBody();
	if (this->getBody().length() > 0)
		buffer << CRLFTWO;
	else
		buffer << CRLF;

	return (buffer.str());
}

void Response::putToResponseMap(int fd)
{
	// i purposly do not check for existance before writing to it so that everything would be overridden if it existed

	// this->_responseMap[fd].buffer = ""; // LEGACY
	// this->_responseMap[fd].header = ""; // LEGACY
	this->_responseMap[fd].response = this->getResponse();
	this->_responseMap[fd].total = this->_responseMap[fd].response.length();
	this->_responseMap[fd].bytesLeft = this->_responseMap[fd].total;
}

// std::string Response::constructHeader(void) // not sure where the other construct header came from, check this!!!!
// {
// 	std::stringstream buffer;
// 	// buffer << this->constructHeader();
// 	buffer << this->getBody();
// 	buffer << CRLFTWO;

// 	return (buffer.str());
// }

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

void Response::createErrorBody(void)
{
	this->body = createErrorString(this->status, this->statusMessage);
}

void Response::createIndex(const Request& request)
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
	<h1 style=\"color:black\">Index of "
	<< request.getDecodedTarget()
	<< "\n\
	</h1>\n\
	</center>\n\
	<div style=\"margin-left:0%\">\n\
	<ul>";
	DIR *dir;
	struct dirent *dirStruct;
	std::string name;
	dir = opendir(request.getRoutedTarget().c_str());
	if (dir)
	{
		while ((dirStruct = readdir(dir)) != NULL)
		{
			name = dirStruct->d_name;
			if (name.compare("..") == 0)
			{
				body << "<li><a href=\"" << name << "\">" << "Parent Directory" << "</a></li>\n";
			}
			else if (name.length() != 0 && name.compare(".") != 0)
			{
				if (dirStruct->d_type == DT_DIR)
					body << "<li><a href=\"" << name << "/\">" << name << "</a></li>\n";
				else
					body << "<li><a href=\"" << name << "\">" << name << "</a></li>\n";
			}
		}
		body <<
		"</div>\n\
		</body>\n\
		</html>";
		this->body = body.str();
		closedir(dir);
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
	// std::cerr << BOLD << RED << "target:" << target << RESET << std::endl;
	if (access(target.c_str(), F_OK) != 0 && errno == EACCES)
		throw ERROR_403();
	if (targetExists(target) == false || staticTargetIsDir(target) == true)
		throw ERROR_404();
	std::ifstream file(target.c_str(), std::ios::binary);
	if (file.is_open())
	{
		// std::cerr << BOLD << RED << "open" << RESET << std::endl;
		tempBody << file.rdbuf();
		file.close();
		this->body = tempBody.str();
	}
	else
	{
		throw ERROR_500();
	}
}

void Response::addContentLengthHeaderField(void)
{
	std::stringstream contentLength;
	// addHeaderField("Server", "localhost:8080");
	// if (headerFields.count("transfer-encoding") == 0)
	// {
		contentLength << this->body.length();
		addHeaderField("content-length", contentLength.str());
	// }
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

const char* Response::ERROR_403::what() const throw()
{
	return ("403");
}

const char* Response::ERROR_423::what() const throw()
{
	return ("423");
}

const char* Response::InvalidProtocol::what() const throw() //AE is it good to have different codes for request/response?
{
	return ("500"); // is Invalid Protocoll really a 500????
}

const char* Response::InternalServerErrorException::what() const throw() // think to unify the Exceptions for errors to be ERROR_XXX
{
	return ("500");
}
const char* Response::ERROR_500::what() const throw()
{
	return ("500");
}

const char* Response::ClientDisconnectException::what() const throw()
{
	return ("client disconnected");
}

bool targetExists(const std::string& target)
{
	struct stat statStruct;

	if (stat(target.c_str(), &statStruct) == 0)
			return (true);
	return (false);
}

const char* Response::SizeTOverflowException::what(void) const throw()
{
	return ("413");
}

const char* Response::NegativeDecimalsNotAllowedException::what(void) const throw()
{
	return ("400");
}

const char* Response::ClientDisconnect::what(void) const throw()
{
	return ("client disconnected");
}

const char* Response::MissingChunkContentLengthException::what(void) const throw()
{
	return ("411");
}



/********** LEGACY CODE BELOW **********/

// int Response::sendall(const int sock_fd, char *buffer, const int len) const
// {
// 	int total;
// 	int bytesleft;
// 	int n;

// 	total = len;
// 	bytesleft = len;
// 	while (total > 0)
// 	{
// 		n = send(sock_fd, buffer, bytesleft, 0);
// 		if (n == -1)
// 		{
// 			perror("send");
// 			return (-1);
// 		}
// 		total -= n;
// 		bytesleft -= n;
// 		buffer += n;
// 	}
// 	close(sock_fd);
// 	#ifdef SHOW_LOG
// 		std::cout << RED << "fd: " << sock_fd << " was closed after sending response" << RESET << std::endl;
// 	#endif
// 	return (0);
// }

// std::string Response::constructChunkedHeader(void)
// {
// 	std::stringstream stream;

// 	stream << this->protocol << " " << this->status << " " << this->statusMessage << CRLF;
// 	// stream << "Content-Type: " << "image/jpg" << CRLF;
// 	stream << "Transfer-Encoding: chunked" << CRLFTWO;

// 	return (stream.str());
// }
