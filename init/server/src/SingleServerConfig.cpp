#include "SingleServerConfig.hpp"

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
	while (buffer.find("{") == std::string::npos && configStream.good()) // maybe change this.....!!!!!!!!!!!!
	{
		buffer.clear();
		std::getline(configStream, buffer);
	}
	buffer.clear();
	while (std::getline(configStream, buffer) && configStream.good() && buffer != "}")
	{
		// !!!!!!!!!!!!!!!!!
		// std::cout << BLUE << buffer << "<-- reached the evaluateKeyValue function" << RESET << std::endl;
		if (buffer.find("location") != std::string::npos)
		{
			std::stringstream locationBlock;
			locationBlock << buffer << std::endl;
			// std::string subkey = buffer.substr(buffer.find_first_of(WHITESPACE));
			// subkey = subkey.substr(0, subkey.find_first_of(WHITESPACE));

			while (std::getline(configStream, buffer) && configStream.good())
			{
				locationBlock << buffer << std::endl;
				if (buffer == "}")
					break ;
			}
			buffer.clear();
			buffer = locationBlock.str();
		}
		this->_parseKeyValue(buffer);
		buffer.clear();
	}
}

void SingleServerConfig::_parseKeyValue(std::string keyValue)
{
	// std::cout << "given to _parseKeyValue >" << GREEN << keyValue << RESET << "<" << std::endl;

	std::string key = keyValue.substr(0, keyValue.find_first_of(WHITESPACE));
	std::string value = "";
	int nKey = 0;
	for (;nKey < not_found; ++nKey)
	{
		if (configVariables[nKey] == key)
			break ;
	}
	switch (nKey)
	{
	case (listen):
	{
		value = keyValue.substr(keyValue.find_last_of(WHITESPACE) + 1);
		this->_checkListen(value);
		this->_conf->listen.push_back(value);
		break ;
	}

	case (root):
	{
		value = keyValue.substr(keyValue.find_last_of(WHITESPACE) + 1);
		this->_conf->root = value;
		break ;
	}

	case (server_name):
	{
		value = keyValue.substr(keyValue.find_last_of(WHITESPACE) + 1);
		this->_conf->serverName = value;
		break;
	}

	case (autoindex):
	{
		value = keyValue.substr(keyValue.find_last_of(WHITESPACE) + 1);
		this->_conf->autoIndex = (value.compare("true") == 0);
		break ;
	}

	case (index_page):
	{
		value = keyValue.substr(keyValue.find_last_of(WHITESPACE) + 1);
		this->_conf->indexPage = value;
		break ;
	}

	case (chunked_transfer):
	{
		value = keyValue.substr(keyValue.find_last_of(WHITESPACE) + 1);
		this->_conf->chunkedTransfer = (value.compare("true") == 0);
		break ;
	}

	case (client_body_buffer_size):
	{
		value = keyValue.substr(keyValue.find_last_of(WHITESPACE) + 1);
		this->_conf->clientBodyBufferSize = this->_atosizet(value.c_str());
		break ;
	}

	case (client_max_body_size):
	{
		value = keyValue.substr(keyValue.find_last_of(WHITESPACE) + 1);
		this->_conf->clientMaxBodySize = this->_atosizet(value.c_str());
		break ;
	}

	case (cgi):
	{
		this->_handleCgi(keyValue);
		break ;
	}

	case (cgi_bin):
	{
		value = keyValue.substr(keyValue.find_last_of(WHITESPACE) + 1);
		this->_conf->cgiBin = value;
		break ;
	}

	case (location):
	{
		this->_handleLocation(keyValue);
		break ;
	}

	case (error_page):
	{
		this->_handleErrorPage(keyValue);
		break ;
	}

	case (log_level):
	{
		value = keyValue.substr(keyValue.find_last_of(WHITESPACE) + 1);
		this->_conf->showLog = (value.compare("true") == 0);
		break ;
	}

	default: // in case of some content that doe not match a known variable
	{
		std::cerr << RED << ">" << keyValue << "<" << std::endl;
		throw SingleServerConfig::InvalidKeyException();
		break;
	}
	}
	// std::cout << YELLOW << "key:\t>" << key << "<" << std::endl << BLUE << "value:\t>" << value << "<" << RESET << std::endl;
}

void SingleServerConfig::_checkListen(std::string value)
{
	size_t port = this->_atosizet(value);
	if (port > USHRT_MAX)
	{
		std::cout << RED << port << RESET << std::endl;
		throw SingleServerConfig::InvalidPortException();
	}
	std::cout << BLUE << "in _checkListen: >" << YELLOW << value << BLUE << "<" RESET << std::endl;
}

void SingleServerConfig::_handleCgi(std::string line)
{
	std::cout << BLUE << "in _handleCgi: >" << YELLOW << line << BLUE << "<" RESET << std::endl;
}

void SingleServerConfig::_handleLocation(std::string block)
{
	std::cout << BLUE << "in _handleLocation: >" << YELLOW << block << BLUE << "<" RESET << std::endl;
}

void SingleServerConfig::_handleErrorPage(std::string line)
{
	std::cout << BLUE << "in _handleErrorPage: >" << YELLOW << line << BLUE << "<" RESET << std::endl;
}

size_t SingleServerConfig::_atosizet(std::string str)
{
	size_t out = 0;
	std::stringstream sizeTMax;
	sizeTMax << SIZE_T_MAX;
	if (str.find("-") != std::string::npos && str.find_first_of(DECIMAL) != std::string::npos && str.find("-") == str.find_first_of(DECIMAL) - 1)
	{
		std::cout << str << std::endl;
		throw SingleServerConfig::NegativeDecimalsNotAllowedException();
	}
	else if (str.find_first_of(DECIMAL) != std::string::npos)
	{
		std::string number = str.substr(str.find_first_of(DECIMAL));
		number = number.substr(0, number.find_first_not_of(DECIMAL));
		if (sizeTMax.str().compare(number) > 0)
		{
			std::cout << RED << number << RESET << std::endl;
			throw SingleServerConfig::SizeTOverflowException();
		}
		else
			std::istringstream(str) >> out;
	}
	return (out);
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

const char* SingleServerConfig::SizeTOverflowException::what(void) const throw()
{
	return ("↑↑↑ this number is too big, stay below 18446744073709551616");
}

const char* SingleServerConfig::NegativeDecimalsNotAllowedException::what(void) const throw()
{
	return ("↑↑↑ no negative decimals allowed as input");
}

const char* SingleServerConfig::InvalidPortException::what(void) const throw()
{
	return ("↑↑↑ this is an invalid value for a port");
}
