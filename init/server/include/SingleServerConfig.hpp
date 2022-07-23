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
#include <vector>
#include <climits>

#include "Base.hpp"
#include "Config.hpp"

// classes
class SingleServerConfig
{
	private:
		ConfigStruct *_conf;
		// std::vector<std::string> _listen;
		// std::string _root;
		// std::string _serverName;
		// bool _autoIndex;
		// std::string _indexPage;
		// bool _chunkedTransfer;
		// size_t _clientBodyBufferSize;
		// size_t _clientMaxBodySize;
		// std::vector<std::string> _cgi;
		// std::string _cgiBin;
		// std::map<std::string, std::string> _location;
		// std::vector<std::string> _errorPage;
		// size_t _logLevel;

	// Private Methods
		void _setVariables(std::string config);
		void _parseKeyValue(std::string);

		unsigned short _checkListen(std::string value);
		// void _handleCgi(std::string line);
		void _handleLocation(std::string line);
		void _handleErrorPage(std::string line);

		size_t _strToSizeT(std::string str);

		std::string _printLocationStruct(LocationStruct locationStruct);
		LocationStruct _fillLocationStruct(std::string block);

	public:
	// Constructors
		SingleServerConfig();
		SingleServerConfig(std::string server, ConfigStruct *conf);

	// Deconstructors
		~SingleServerConfig();

	// Public Methods



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

		class SizeTOverflowException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class NegativeDecimalsNotAllowedException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class InvalidPortException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class InvalidWhitespaceException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class InvalidValueTypeException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class NotAnErrorCodeException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class DuplicateErrorPageException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class DefaultNotAllowedException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class NoValueFoundException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class InvalidLocationBlockException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class DuplicateLocationException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};
};

#endif // SINGLESERVER_HPP
