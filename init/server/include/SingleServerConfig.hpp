// Header-protection
#ifndef SINGLESERVER_HPP
#define SINGLESERVER_HPP
#pragma once

// Includes
#include <string>
#include <iostream>
#include <utility>
#include <sstream>
#include <stdbool.h>
#include <map>

#include "Base.hpp"

// classes

class SingleServerConfig
{
	private:
		bool _isLocation;

		std::string *_listen;
		std::string _serverName;
		std::string _root;
		std::string _index;
		std::map<std::string, std::string *> _location;

	// Private Methods
		void _setVariables(std::string config);
		void _evaluateKeyValue(std::string);
		void _checkVariables();

	public:
	// Constructors
		SingleServerConfig();
		SingleServerConfig(std::string server);

	// Deconstructors
		~SingleServerConfig();

	// Public Methods

	// Getter
		const std::string *getListen() const;
		const std::string getServerName() const;
		const std::string getRoot() const;
		const std::string getIndex() const;
		const std::string *getLocation(std::string to_find) const;

	// Exceptions
		class NoRootException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class NoIndexException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class NoPortException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class NotAPortException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class InvalidKeyException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};
};
#endif // SINGLESERVER_HPP
