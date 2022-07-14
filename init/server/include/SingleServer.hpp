// Header-protection
#ifndef SINGLESERVER_HPP
#define SINGLESERVER_HPP
#pragma once

// Includes
#include <string>
#include <iostream>
#include <utility>
#include <sstream>

// classes

class SingleServer
{
	private:
		std::string *_listen;
		std::string _serverName;
		std::string _root;
		std::string _index;
		std::string _cgi_param;
		std::pair<std::string, std::string> _location; // check if this is good, it is for the cgi stuff

	public:
	// Constructors
		SingleServer(std::string server);

	// Deconstructors
		~SingleServer();

	// Public Methods

	// Getter
	const std::string *getListen() const;
	const std::string getServerName() const;

};
#endif // SINGLESERVER_HPP
