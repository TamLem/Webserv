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

#include "Base.hpp"

// classes

class SingleServerConfig
{
	private:
		std::vector<std::string> _listen;
		std::string _root;
		std::string _serverName;
		bool _autoIndex;
		std::string _indexPage;
		bool _chunkedTransfer;
		size_t _clientBodyBufferSize;
		size_t _clientMaxBodySize;
		std::vector<std::string> _cgi;
		std::string _cgiBin;
		std::map<std::string, std::string> _location;
		std::vector<std::string> _errorPage;
		size_t _logLevel;

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
		const SingleServerConfig *getConfigAddress() const; // not needed i think !!!!!
		const std::vector<std::string> getListen() const;
		const std::string getRoot() const;
		const std::string getServerName() const;
		bool getAutoIndex() const;
		const std::string getIndexPage() const;
		bool getChunkedTransfer() const;
		size_t getClientBodyBufferSize() const;
		size_t getClientMaxBodySize() const;
		const std::vector<std::string> getCgi() const;
		const std::string getCgiBin() const;
		const std::map<std::string, std::string> getLocation() const;
		const std::vector<std::string> getErrorPage() const; // rethink this type !!!!! maybe map is better, but then more pasing needs to be done
		size_t getLogLevel() const;

	// Getters for printing (consverts every value to string)
		const std::string strGetListen() const;
		const std::string strGetRoot() const;
		const std::string strGetServerName() const;
		const std::string strGetAutoIndex() const;
		const std::string strGetIndexPage() const;
		const std::string strGetChunkedTransfer() const;
		const std::string strGetClientBodyBufferSize() const;
		const std::string strGetClientMaxBodySize() const;
		const std::string strGetCgi() const;
		const std::string strGetCgiBin() const;
		const std::string strGetLocation() const;
		const std::string strGetErrorPage() const;
		const std::string strGetLogLevel() const;

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
// Ostream overload
std::ostream &operator<<(std::ostream &o, SingleServerConfig obj);

#endif // SINGLESERVER_HPP
