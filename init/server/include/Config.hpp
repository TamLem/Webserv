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

#include "SingleServer.hpp"

// classes

class Config
{
	private:
		std::ifstream	_configFile;
		std::string		_configPath;

		std::map<std::string, SingleServer>* _cluster;


	// Private Methods
		void _openConfigFile();
		void _readConfigFile();

	public:
	// Constructors
		Config();
		Config(std::string configPath);

	// Deconstructors
		~Config();

	// Public Methods
		void start(std::string configPath);

	// Getter
		const std::string getConfigPath() const;
		std::map<std::string, SingleServer> *getCluster() const;

	// private: //maybe private for the setters adds more security
	// Setter
		void setConfigPath(std::string configPath);

	public:
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
};
	// Ostream Overload
		std::ostream &operator<<(std::ostream &o, Config *config);

#endif