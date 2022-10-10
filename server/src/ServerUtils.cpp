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

std::string percentDecodingFix(std::string target)
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
			if (isdigit(str[i + 1]) != true || isdigit(str[i + 2]) != true)
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
	return (percentDecodingFix(tmp.str()));
}

void Server::checkLocationMethod(const Request& request) const
{
	if (this->_currentLocationKey.empty() == true)
		return ;
	if (this->_currentConfig.location.find(_currentLocationKey)->second.allowedMethods.count(request.getMethod()) != 1)
		throw MethodNotAllowed();
}

std::list<std::string> Server::splitStringOnDelimiter(const std::string& str, const std::string& delimiter)
{
	std::list<std::string> list;
	size_t last = 0;
	size_t next = 0;
	while ((next = str.find(delimiter, last)) != std::string::npos)
	{
		list.push_back(str.substr(last, next-last));
		last = next + delimiter.length();
	}
	list.push_back(str.substr(last));
	return (list);
}

std::string Server::resoluteTarget(const std::string& input)
{
	std::string target;
	std::list<std::string> tokens;
	bool isDir = false;

	if (*input.rbegin() == '/')
		isDir = true;

	tokens = splitStringOnDelimiter(input, "/");

	
	std::list<std::string>::iterator it = tokens.begin();
	while (it != tokens.end())
	{
		if ((*it).empty() == true || (*it) == ".")
			it = tokens.erase(it);
		else if ((*it) == "..")
		{
			it = tokens.erase(it);
			if (it != tokens.begin())
			{
				it--;
				it = tokens.erase(it);
			}
		}
		else
			it++;
	}

	it = tokens.begin();
	for (; it != tokens.end(); ++it)
	{
		target += "/";
		target += *it;
	}

	if (isDir)
		target += "/";

	if (*input.begin() != '/')
		target = "/" + target;

	return (target);
}

bool Server::_isCgiRequest(const Request& request) const// AE this function has to be included in locationMatching
{
	// if(request.getMethod() != "POST") //remove, to allow other cgi requests
	// 	return (false);
	std::string target = request.getDecodedTarget();
	size_t targetlen = target.length();
	if (this->_currentConfig.cgi.size() != 0)
	{
		std::map<std::string, std::string>::const_iterator it = this->_currentConfig.cgi.begin();
		for (; it != this->_currentConfig.cgi.end(); ++it)
		{
			std::string ext = it->first;
			size_t extlen = ext.length();
			if(targetlen - target.find_last_of('/') > extlen + 1
			&& target.substr(targetlen - extlen, extlen) == it->first)
				return (true);
		}
	}
	return (false);
}

std::string createErrorString(std::string statusCode, std::string statusMessage)
{
	std::stringstream message;
	std::string errorImage;

	if (statusCode < "500")
		errorImage = CLIENT_ERROR_IMG;
	else
		errorImage = SERVER_ERROR_IMG;

	message <<
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
	<img src=\"" <<
	errorImage <<
	"\" align=\"TOP\">\n\
	</center>\n\
	</body>\n\
	</html>";

	return (message.str());
}