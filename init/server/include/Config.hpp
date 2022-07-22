// Header-protection
#ifndef CONFIG_HPP
#define CONFIG_HPP
#pragma once

// Includes
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <stdexcept>

#include "Base.hpp"
#include "SingleServerConfig.hpp"

// classes
class Config
{
	private:
		std::ifstream	_configFile;
		std::string		_configPath;

		std::map<std::string, ConfigStruct> _cluster;
		ConfigStruct _conf; // only a temp used for printing and getters


	// Private Methods
		void _openConfigFile();
		void _readConfigFile();
		void _checkBrackets(std::string buffer);
		void _parseServerBlock(std::string serverBlock);
		void _createConfigStruct(std::string server);
		ConfigStruct _initConfigStruct();
		void _freeConfigStruct();

	public:
	// Constructors
		Config();
		Config(std::string configPath);

	// Deconstructors
		~Config();

	// Public Methods
		void start(std::string configPath);

	// Getter
		// const std::string getConfigPath() const;
		// std::map<std::string, ConfigStruct> getCluster() const;
	
	ConfigStruct getConfig(std::string serverName) const; // new
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
		const std::map<std::string, LocationStruct> getLocation() const;
		const std::map<std::string, std::string> getErrorPage() const; // rethink this type !!!!! maybe map is better, but then more pasing needs to be done
		bool getShowLog() const;

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
		const std::string strGetShowLog() const;

	// private: //maybe private for the setters adds more security
	// Setter
		void setConfigPath(std::string configPath);
		bool applyConfig(std::string serverName);

	// public:
	// Exceptions
		class InvalidBracketsException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class FileOpenException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class ServerInsideServerException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class InvalidCharException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class NoServerNameException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class WrongConfigSyntaxException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class DuplicateServerNameException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class WrongListenBlockException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class ContentOutsideServerBlockException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};
};

// Ostream overload
std::ostream &operator<<(std::ostream &o, Config *a);

#endif