#include "Utils.hpp"
#include "Server.hpp"

bool Server::_crlftwoFound()
{
	if (this->_requestHead.find(CRLFTWO) != std::string::npos)
		return (true);
	else
		return (false);
}

bool Server::_isPrintableAscii(char c)
{
	if (c > 126 || c < 0)
		return (false);
	else
		return (true);
}

std::string staticReplaceInString(std::string str, std::string tofind, std::string toreplace)
{
		size_t position = 0;
		for ( position = str.find(tofind); position != std::string::npos; position = str.find(tofind,position) )
		{
				str.replace(position , tofind.length(), toreplace);
		}
		return(str);
}

std::string staticPercentDecodingFix(std::string target)
{
	std::string accent;
	accent += (const char)204;
	accent += (const char)136;

	std::string ü;
	ü += (const char)195;
	ü += (const char)188;

	std::string ä;
	ä += (const char)195;
	ä += (const char)164;

	std::string ö;
	ö += (const char)195;
	ö += (const char)182;

	target = staticReplaceInString(target, "u" + accent, ü);
	target = staticReplaceInString(target, "a" + accent, ä);
	target = staticReplaceInString(target, "o" + accent, ö);
	return (target);
}

std::string Server::percentDecoding(const std::string& str)
{
	std::stringstream tmp;
	char c;
	int i = 0;
	while (str[i] != '\0')
	{
		if (str[i] == '%')
		{
			if (str[i + 1] == '\0' || str[i + 2] == '\0')
				throw InvalidHex();
			c = char(strtol(str.substr(i + 1, 2).c_str(), NULL, 16));
			if (c == 0)
				throw InvalidHex();
			tmp << c;
			i += 3;
		}
		else
		{
			tmp << str[i];
			i++;
		}
	}
	return (staticPercentDecodingFix(tmp.str()));
}

void Server::checkLocationMethod(const Request& request) const
{
	if (this->_currentLocationKey.empty() == true)
		return ;
	//print  the request method
	std::cout << BLUE << "In check method " << request.getMethod() << RESET << std::endl;
	if (this->_currentConfig.location.find(_currentLocationKey)->second.allowedMethods.count(request.getMethod()) != 1)
		throw MethodNotAllowed();
}

bool Server::_isCgiRequest(std::string requestHead) // AE this function ahs to be included in locationMatching
{
	requestHead = requestHead.substr(0, requestHead.find("HTTP/1.1")); // AE is formatting checked before?
	// if (requestHead.find("/cgi/") != std::string::npos) // AE file extension should deterime if something is cgi or not
	// 	return (true);
	/*if (this->_currentConfig.cgiBin.length() != 0 && requestHead.find(this->_currentConfig.cgiBin) != std::string::npos)
		return (true);
	else */if (this->_currentConfig.cgi.size() != 0)
	{
		std::map<std::string, std::string>::const_iterator it = this->_currentConfig.cgi.begin();
		for (; it != this->_currentConfig.cgi.end(); ++it)
		{
			if (requestHead.find(it->first) != std::string::npos) // AE define this better (has to be ending, not just existing)
				return (true);
		}
	}
	return (false);
}

std::string createErrorString(std::string statusCode, std::string statusMessage)
{
	std::stringstream message;

	message << \
	"<html>\n\
	<head>\n\
	<title>Error " << statusCode << "</title>\n\
	<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"images/favicon.ico\">\n\
	</head>\n\
	<body bgcolor=\"000000\">\n\
	<center>\n\
	<h1 style=\"color:white\">Error " << statusCode << "</h1>\n\
	<p style=\"color:white\">" << statusMessage << "!\n\
	<br><br>\n\
	<img src=\"/images/error.jpg\" align=\"TOP\">\n\
	</center>\n\
	</body>\n\
	</html>";

	return (message.str());
}