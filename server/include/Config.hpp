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
// #include <unistd.h>
// #include <cstring>
// #include <stdio.h>
// #include <stdlib.h>

#include "Base.hpp"
#include "SingleServerConfig.hpp"

// classes
class Config
{
	private:
		Config(const Config&);
		Config& operator=(const Config&);

		std::ifstream	_configFile;
		std::string		_configPath;

		std::map<std::string, ConfigStruct> _cluster;
		ConfigStruct _conf; // only a temp used for printing and getters


	// Private Methods
		void _openConfigFile();
		void _checkBrackets(std::string buffer);
		void _parseServerBlock(std::string serverBlock);
		void _createConfigStruct(std::string server);
		ConfigStruct _initConfigStruct();
		void _readConfigFile();
		const std::string _printLocationStruct(LocationStruct locationStruct) const;

	public:
	// Constructors
		Config();

	// Deconstructors
		~Config();

	// Public Methods
		void start(std::string configPath);
		void printCluster();

	// Getter
		std::map<std::string, ConfigStruct> getCluster();
		const ConfigStruct& getConfigStruct(std::string hostName);

		const std::map<std::string, unsigned short> getListen() const;
		const std::string getRoot() const;
		const std::string getServerName() const;
		bool getAutoIndex() const;
		const std::string getIndexPage() const;
		size_t getClientBodyBufferSize() const;
		size_t getClientMaxBodySize() const;
		// const std::vector<std::string> getCgi() const; // only needed if we do bonus
		const std::string getCgiBin() const;
		const std::map<std::string, LocationStruct> getLocation() const;
		const std::map<std::string, std::string> getErrorPage() const;

	// Getters for printing (converts every value to string)
		const std::string strGetListen() const;
		const std::string strGetRoot() const;
		const std::string strGetServerName() const;
		const std::string strGetAutoIndex() const;
		const std::string strGetIndexPage() const;
		const std::string strGetClientBodyBufferSize() const;
		const std::string strGetClientMaxBodySize() const;
		// const std::string strGetCgi() const; // only needed if we do bonus
		const std::string strGetCgiBin() const;
		const std::string strGetLocation() const;
		const std::string strGetErrorPage() const;

	// private: //maybe private for the setters adds more security
	// Setter
		void setConfigPath(std::string configPath); // set to private??????
		bool applyConfig(std::string serverName); // check if moving it to private makes sense

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

		class NoServerFoundException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};
};

#endif