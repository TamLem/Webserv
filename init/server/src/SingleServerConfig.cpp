#include "SingleServerConfig.hpp"


// Constructors
SingleServerConfig::SingleServerConfig()
{
	std::cout << RED << "SingleServerConfig default constructor called" << RESET << std::endl;
}
SingleServerConfig::SingleServerConfig(std::string config)
{
	std::cout << RED << "SingleServerConfig constructor called" << RESET << std::endl;
	std::cout << "This content reached the server:\n>" << config << "<" << std::endl;
	this->_setVariables(config);
}

// Deconstructors
SingleServerConfig::~SingleServerConfig()
{
	std::cout << RED << "SingleServerConfig deconstructor for " << this->_serverName << " called:" << this << RESET << std::endl;
}

enum
{
	listen,
	root,
	server_name,
	autoindex,
	index_page,
	chunked_transfer,
	client_body_in_single_buffer, // check if needed
	client_body_buffer_size,
	client_max_body_size,
	cgi,
	cgi_bin,
	location,
	error_page,
	log_level,
	not_found
};

std::string configVariables[]=
{
	"listen",
	"root",
	"server_name",
	"autoindex",
	"index_page",
	"chunked_transfer",
	"client_body_in_single_buffer", // check if needed
	"client_body_buffer_size",
	"client_max_body_size",
	"cgi",
	"cgi_bin",
	"location",
	"error_page", // can be i.e. value 404 /404.html || value 500 502 503 504 /50x.html
	"log_level"
};

void SingleServerConfig::_setVariables(std::string config)
{
	std::stringstream configStream(config);

	std::string buffer = "";
	while (buffer.find("{") == std::string::npos && configStream.good())
	{
		buffer.clear();
		std::getline(configStream, buffer);
	}
	buffer.clear();
	while (configStream.good() && buffer != "}")
	{
		std::getline(configStream, buffer);
		std::cout << BLUE << buffer << "<-- reached the evaluateKeyValue function" << RESET << std::endl;
		this->_evaluateKeyValue(buffer);
	}
}

void SingleServerConfig::_evaluateKeyValue(std::string keyValue)
{
	std::string key = keyValue.substr(0, keyValue.find_first_of(WHITESPACE));
	std::string value = "";
	if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
	{
		std::cout << "location found" << std::endl;
		// consider a map for the storage of the location
	}
	else
		value = keyValue.substr(keyValue.find_last_of(WHITESPACE) + 1);
	int nKey = 0;
	for (;nKey < log_level; ++nKey)
	{
		// if (nKey > log_level)
		// {
		// 	std::cerr << RED << ">" << key;
		// 	throw SingleServerConfig::InvalidKeyException();
		// }
		if (configVariables[nKey] == key)
			break ;
	}
	switch (nKey)
	{
	case (server_name):
		this->_serverName = value;
		break;

	// default:
	case (not_found):
		std::cerr << RED << ">" << key << "<" << std::endl;
		throw SingleServerConfig::InvalidKeyException();
		break;
	}
	std::cout << YELLOW << "key:\t>" << key << "<" << std::endl << BLUE << "value:\t>" << value << "<" << std::endl;
}

// Public Methods

// Getter
const std::string SingleServerConfig::getServerName() const
{
	return (this->_serverName);
}

// Setter

// Exceptions
const char* SingleServerConfig::NoRootException::what(void) const throw()
{
	return ("'root' is mandatory for config file");
}

const char* SingleServerConfig::NoIndexException::what(void) const throw()
{
	return ("'index_page' is mandatory for config file");
}

const char* SingleServerConfig::NoPortException::what(void) const throw()
{
	return ("'listen' is mandatory for config file");
}

const char* SingleServerConfig::NotAPortException::what(void) const throw()
{
	return ("invalid port in .conf file found, see above ↑");
}

const char* SingleServerConfig::InvalidKeyException::what(void) const throw()
{
	return ("invalid key for the .conf file found, see above ↑");
}
