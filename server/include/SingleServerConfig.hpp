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
	// defines only to not have undefined behaviour
		SingleServerConfig(const SingleServerConfig&);
		SingleServerConfig& operator=(const SingleServerConfig&);
		SingleServerConfig(void);

	// private Members
		ConfigStruct *_conf;

	// Private Methods
		void _checkConfigStruct();
		void _setVariables(std::string config);
		void _parseKeyValue(std::string);

		void _handleListen(std::string keyValue);
		unsigned short _checkListen(std::string value);
		// void _handleCgi(std::string line); // only needed if we do bonus
		void _handleLocation(std::string line);
		void _handleErrorPage(std::string line);

		size_t _strToSizeT(std::string str);

		std::string _printLocationStruct(LocationStruct locationStruct);
		LocationStruct _fillLocationStruct(std::string block);
		LocationStruct _initLocationStruct();
		// void _handleMethod();
		// void _handle

	public:
	// Constructors
		// SingleServerConfig();
		SingleServerConfig(std::string server, ConfigStruct *conf);

	// Deconstructors
		~SingleServerConfig();

	// Public Methods

	// Exceptions
		// class NoRootException : public std::exception
		// {
		// 	public:
		// 		virtual const char* what() const throw();
		// };

		// class NoIndexException : public std::exception
		// {
		// 	public:
		// 		virtual const char* what() const throw();
		// };

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

		class InvalidKeyValueException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class InvalidMethodValueException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class InvalidLocationException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class DuplicateLocationRootException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class DuplicateLocationIndexException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class DuplicateLocationAutoIndexException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class InvalidIndexPageException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class MissingIndexException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class InvalidPathException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};
};

#endif // SINGLESERVER_HPP