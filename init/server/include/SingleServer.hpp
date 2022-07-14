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
		std::pair<std::string, std::string> *_location;

	// Private Methods
		void setVariables(std::string server);
		void checkVariables();

	public:
	// Constructors
		SingleServer(std::string server);

	// Deconstructors
		~SingleServer();

	// Public Methods

	// Getter
		const std::string *getListen() const;
		const std::string getServerName() const;
		const std::string getRoot() const;
		const std::string getIndex() const;
		const std::pair<std::string, std::string>* getLocation() const;

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
};
#endif // SINGLESERVER_HPP
