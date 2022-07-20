#include "SingleServerConfig.hpp"

#include <stdlib.h> // forbidden atoi is connected to this !!!!!!!!

// Constructors
SingleServerConfig::SingleServerConfig()
{
	std::cout << GREEN << "SingleServerConfig default constructor called for " << this << RESET << std::endl;
}
SingleServerConfig::SingleServerConfig(std::string config, ConfigStruct *conf): _conf(conf)
{
	std::cout << GREEN << "SingleServerConfig constructor called for " << this << RESET << std::endl;
	// std::cout << "This content reached the server:\n>" << config << "<" << std::endl;
	this->_setVariables(config);
}

// Deconstructors
SingleServerConfig::~SingleServerConfig()
{
	std::cout << RED << "SingleServerConfig deconstructor called for " << this << RESET << std::endl;
}

enum
{
	listen,
	root,
	server_name,
	autoindex,
	index_page,
	chunked_transfer,
	client_body_in_single_buffer, // check if needed, not implemented !!!!!!!
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

// Private Methods
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
	while (std::getline(configStream, buffer) && configStream.good() && buffer != "}")
	{
		!!!!!!!!!!!!!!!!!
		// std::cout << BLUE << buffer << "<-- reached the evaluateKeyValue function" << RESET << std::endl;
		// std::stringstream locationBlock;
		// if (buffer.find("location") != std::string::npos)
		// {
		// 	std::string subkey = buffer.substr(buffer.find_first_of(WHITESPACE));
		// 	subkey = subkey.substr(0, subkey.find_first_of(WHITESPACE));

		// 	while (std::getline(configStream, buffer) && configStream.good() && buffer != "}")
		// 	{
		// 		std::cout << buffer << std::endl;
		// 	}
		// // 	locationBlock << buffer;
		// }
		// else
			this->_parseKeyValue(buffer);
		buffer.clear();
	}
	// std::cout << "done" << std::endl;
}

void SingleServerConfig::_parseKeyValue(std::string keyValue)
{
	std::string key = keyValue.substr(0, keyValue.find_first_of(WHITESPACE));
	std::string value = "";
	// if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
	// {
	// 	// std::cout << "location found" << std::endl;
	// 	// consider a map for the storage of the location
	// }
	// else
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
	case (listen):
	{
		// std::cout << "value listen: >" << YELLOW << value << RESET << "<" << std::endl;
		this->_conf->listen.push_back(value);
		break ;
	}

	case (root):
	{
		this->_conf->root = value;
		break ;
	}

	case (server_name):
	{
		this->_conf->serverName = value;
		break;
	}

	case (autoindex):
	{
		this->_conf->autoIndex = (value.compare("true") == 0);
		break ;
	}

	case (index_page):
	{
		this->_conf->indexPage = value;
		break ;
	}

	case (chunked_transfer):
	{
		this->_conf->chunkedTransfer = (value.compare("true") == 0);
		break ;
	}

	case (client_body_buffer_size):
	{
		this->_conf->clientBodyBufferSize = atoi(value.c_str()); // check if forbidden!!!!!!!!
		break ;
	}

	case (client_max_body_size):
	{
		this->_conf->clientMaxBodySize = atoi(value.c_str()); //check if forbidden !!!!!!!!!!!
		break ;
	}

	case (cgi):
	{
		// this->_conf->cgi.push_back(value); // this value needs to be checked again!!!!!!! it is wrong
		// std::cout << "value cgi: >" << YELLOW << value << RESET << "<" << std::endl;
		this->_handleCgi(value);
		break ;
	}

	case (cgi_bin):
	{
		this->_conf->cgiBin = value;
		break ;
	}

	case (location): // fix this!!!!!!!!!!!!!!!!!!!!!!
	{
		// std::cout << "value location: >" << YELLOW << value << RESET << "<" << std::endl;
		// std::string first_arg = "first_arg";
		// LocationStruct buffer;
		// buffer.deleteAllowed = true;
		// buffer.getAllowed = true;
		// buffer.postAllowed = true;
		// buffer.indexPage = value;
		// this->_conf->location.insert(std::make_pair<std::string, LocationStruct>(first_arg, buffer)); // this is also still wrong
		this->_handleLocation(value);
		break ;
	}

	case (error_page):
	{
		this->_conf->errorPage.push_back(value); // still wrong!!!!!!!!!!
		break ;
	}

	case (log_level):
	{
		this->_conf->showLog = (value.compare("true") == 0);
		break ;
	}

	// case (not_found):
	default:
	{
		std::cerr << RED << ">" << key << "<" << std::endl;
		throw SingleServerConfig::InvalidKeyException();
		break;
	}
	}
	// std::cout << YELLOW << "key:\t>" << key << "<" << std::endl << BLUE << "value:\t>" << value << "<" << RESET << std::endl;
}

void SingleServerConfig::_handleListen(std::string line)
{
	std::cout << "in _handleListen: >" << YELLOW << line << RESET << std::endl;
}

void SingleServerConfig::_handleCgi(std::string line)
{
	std::cout << "in _handleCgi: >" << YELLOW << line << RESET << std::endl;
}

void SingleServerConfig::_handleLocation(std::string line)
{
	std::cout << "in _handleLocation: >" << YELLOW << line << RESET << std::endl;
}

void SingleServerConfig::_handleErrorPage(std::string line)
{
	std::cout << "in _handleErrorPage: >" << YELLOW << line << RESET << std::endl;
}

// Public Methods

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
	return ("↑↑↑ invalid port in .conf file found, see above");
}

const char* SingleServerConfig::InvalidKeyException::what(void) const throw()
{
	return ("↑↑↑ invalid key for the .conf file found, see above");
}

