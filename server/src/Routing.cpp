#include "Server.hpp"

int Server::routeFile(Request& request, std::map<std::string, LocationStruct>::const_iterator it, const std::string& target)
{
	std::string result;
	if (isExtension(target, it->first.substr(1)) == true)
	{
		if (it->second.root.empty())
			result = this->_currentConfig.root;
		else
			result = it->second.root;
		result += target.substr(target.find_last_of('/') + 1);
		request.setRoutedTarget("." + result);
		_currentLocationKey = it->first;
		#ifdef SHOW_LOG_ROUTING
			std::cout  << YELLOW << "FILE ROUTING RESULT!: " << request.getRoutedTarget() << " for location: " << _currentLocationKey << RESET << std::endl;
		#endif
		return (0);
	}
	return (1);
}

void Server::routeDir(Request& request, std::map<std::string, LocationStruct>::const_iterator it, const std::string& target, int& max_count)
{
	std::string path;
	std::string result;

	path = it->first;
	#ifdef SHOW_LOG_2
		std::cout  << BLUE << "path: " << path << RESET << std::endl;
	#endif
	int i = 0;
	int segments = 0;
	if (target.length() >= path.length())
	{
		while (path[i] != '\0')
		{
			if (path[i] != target[i])
			{
				segments = 0;
				break ;
			}
			if (path[i] == '/')
				segments++;
			i++;
		}
		if (target[i - 1] != '\0' && target[i - 1] != '/')
			segments = 0;
	}
	if (segments > max_count)
	{
		max_count = segments;
		if (it->second.root.empty())
			result = this->_currentConfig.root;
		else
			result = it->second.root;
		result += target.substr(i);
		if (*result.rbegin() == '/')
		{
			request.isFile = false;
			if (it->second.indexPage.empty() == false)
				request.indexPage = it->second.indexPage;
			else
				request.indexPage = this->_currentConfig.indexPage;
		}
		request.setRoutedTarget("." + result);
		_currentLocationKey = it->first;
		#ifdef SHOW_LOG_ROUTING
			std::cout  << YELLOW << "DIR MATCH!: " << request.getRoutedTarget() << " for location: " << _currentLocationKey << RESET << std::endl;
		#endif
	}
}

void Server::routeDefault(Request& request)
{
	std::string result;

	result = this->_currentConfig.root + request.getDecodedTarget().substr(1);
	if (*result.rbegin() == '/')
	{
		request.isFile = false;
		request.indexPage = this->_currentConfig.indexPage;
	}
	request.setRoutedTarget("." + result);
	_currentLocationKey = "";
	#ifdef SHOW_LOG_ROUTING
		std::cout  << YELLOW << "DEFAULT " << RESET ;
	#endif
}

void Server::matchLocation(Request& request)
{
	int max_count = 0;
	std::string target = request.getDecodedTarget();
	#ifdef SHOW_LOG_2
	std::cout  << RED << "target: " << target << RESET << std::endl;
	for (std::map<std::string, LocationStruct>::const_iterator it = this->_currentConfig.location.begin(); it != this->_currentConfig.location.end(); ++it)
	{
		std::cout << RED << it->first << ": "
		<< it->second.root << " is dir: " << it->second.isDir << RESET << "\n";
	}
	#endif
	for (std::map<std::string, LocationStruct>::const_iterator it = this->_currentConfig.location.begin(); it != this->_currentConfig.location.end(); ++it)
	{
		if (it->second.isDir == false)
		{
			if (routeFile(request, it, target) == 0)
				return ;
		}
		if (it->second.isDir == true)
			routeDir(request, it, target, max_count);
	}
	if (max_count == 0)
		routeDefault(request);
	#ifdef SHOW_LOG_ROUTING
		std::cout  << YELLOW << "DIR ROUTING RESULT!: " << request.getRoutedTarget() << " for location: " << _currentLocationKey  << RESET << std::endl;
	#endif
}
