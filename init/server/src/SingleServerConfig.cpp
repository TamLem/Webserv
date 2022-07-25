#include "SingleServerConfig.hpp"

// Constructors
SingleServerConfig::SingleServerConfig()
{
	#ifdef SHOW_LOG
		std::cout << GREEN << "SingleServerConfig default constructor called for " << this << RESET << std::endl;
	#endif
}
SingleServerConfig::SingleServerConfig(std::string config, ConfigStruct *conf): _conf(conf)
{
	#ifdef SHOW_LOG
		std::cout << GREEN << "SingleServerConfig constructor called for " << this << RESET << std::endl;
	#endif
	// std::cout << "This content reached the server:\n>" << config << "<" << std::endl;
	this->_setVariables(config);
}

// Deconstructors
SingleServerConfig::~SingleServerConfig()
{
	#ifdef SHOW_LOG
		std::cout << RED << "SingleServerConfig deconstructor called for " << this << RESET << std::endl;
	#endif
}

enum
{
	listen,
	root,
	server_name,
	autoindex,
	index_page,
	// chunked_transfer,
	client_body_buffer_size,
	client_max_body_size,
	// cgi, //only needed if we do bonus
	cgi_bin,
	location,
	error_page,
	// show_log,
	not_found
};

std::string configVariables[]=
{
	"listen",
	"root",
	"server_name",
	"autoindex",
	"index_page",
	// "chunked_transfer",
	"client_body_buffer_size",
	"client_max_body_size",
	// "cgi", // only needed if we do bonus
	"cgi_bin",
	"location",
	"error_page" // can be i.e. value 404 /404.html || value 500 502 503 504 /50x.html
	// "show_log"
};

// Private Methods
void SingleServerConfig::_setVariables(std::string config)
{
	#ifdef SHOW_LOG_2
		std::cout << BLUE << config << "<-- reached the _setVariables function" << RESET << std::endl;
	#endif
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
	#ifdef SHOW_LOG_2
		std::cout << "given to _parseKeyValue >" << GREEN << keyValue << RESET << "<" << std::endl;
	#endif

	if (keyValue.find_first_of(WHITESPACE) == std::string::npos)
	{
		std::cout << RED << keyValue << RESET << std::endl;
		throw SingleServerConfig::NoValueFoundException();
	}
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
		if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
		{
			std::cout << RED << keyValue << std::endl;
			throw SingleServerConfig::InvalidWhitespaceException();
		}
		value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
		if (value.find_first_not_of(DECIMAL) != std::string::npos)
		{
			std::cout << RED << keyValue << RESET << std::endl;
			throw SingleServerConfig::NotAPortException();
		}
		unsigned short port = this->_checkListen(value);
		if (this->_conf->listen.count(value) == 0)
		{
			this->_conf->listen.insert(std::make_pair<std::string, unsigned short>(value, port));
		}
		break ;
	}

	case (root):
	{
		if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
		{
			std::cout << RED << keyValue << std::endl;
			throw SingleServerConfig::InvalidWhitespaceException();
		}
		value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
		this->_conf->root = value;
		break ;
	}

	case (server_name):
	{
		if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
		{
			std::cout << RED << keyValue << std::endl;
			throw SingleServerConfig::InvalidWhitespaceException();
		}
		value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
		if (value == "default")
			throw SingleServerConfig::DefaultNotAllowedException();
		this->_conf->serverName = value;
		break;
	}

	case (autoindex):
	{
		if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
		{
			std::cout << RED << keyValue << std::endl;
			throw SingleServerConfig::InvalidWhitespaceException();
		}
		value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
		if (value != "true" && value != "false")
		{
			std::cout << RED << keyValue << std::endl;
			throw SingleServerConfig::InvalidValueTypeException();
		}
		this->_conf->autoIndex = (value.compare("true") == 0);
		break ;
	}

	case (index_page):
	{
		if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
		{
			std::cout << RED << keyValue << std::endl;
			throw SingleServerConfig::InvalidWhitespaceException();
		}
		value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
		this->_conf->indexPage = value;
		break ;
	}

	// case (chunked_transfer):
	// {
	// 	if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
	// 	{
	// 		std::cout << RED << keyValue << std::endl;
	// 		throw SingleServerConfig::InvalidWhitespaceException();
	// 	}
	// 	value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
	// 	if (value != "true" && value != "false")
	// 	{
	// 		std::cout << RED << keyValue << std::endl;
	// 		throw SingleServerConfig::InvalidValueTypeException();
	// 	}
	// 	this->_conf->chunkedTransfer = (value.compare("true") == 0);
	// 	break ;
	// }

	case (client_body_buffer_size):
	{
		if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
		{
			std::cout << RED << keyValue << std::endl;
			throw SingleServerConfig::InvalidWhitespaceException();
		}
		value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
		if (value.find_first_not_of(DECIMAL) != std::string::npos)
		{
			std::cout << RED << keyValue << std::endl;
			throw SingleServerConfig::InvalidValueTypeException();
		}
		this->_conf->clientBodyBufferSize = this->_strToSizeT(value.c_str());
		break ;
	}

	case (client_max_body_size):
	{
		if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
		{
			std::cout << RED << keyValue << std::endl;
			throw SingleServerConfig::InvalidWhitespaceException();
		}
		value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
		if (value.find_first_not_of(DECIMAL) != std::string::npos)
		{
			std::cout << RED << keyValue << std::endl;
			throw SingleServerConfig::InvalidValueTypeException();
		}
		this->_conf->clientMaxBodySize = this->_strToSizeT(value.c_str());
		break ;
	}

	// case (cgi): // only needed if we do bonus, else will be standard, ask tam!!!!!!!!!!
	// {
	// 	this->_handleCgi(keyValue);
	// 	break ;
	// }

	case (cgi_bin):
	{
		if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
		{
			std::cout << RED << keyValue << std::endl;
			throw SingleServerConfig::InvalidWhitespaceException();
		}
		value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
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

	// case (show_log):
	// {
	// 	if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
	// 	{
	// 		std::cout << RED << keyValue << std::endl;
	// 		throw SingleServerConfig::InvalidWhitespaceException();
	// 	}
	// 	value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
	// 	if (value != "true" && value != "false")
	// 	{
	// 		std::cout << RED << keyValue << std::endl;
	// 		throw SingleServerConfig::InvalidValueTypeException();
	// 	}
	// 	this->_conf->showLog = (value.compare("true") == 0);
	// 	break ;
	// }

	default: // in case of some content that does not match a known variable
	{
		std::cerr << RED << ">" << keyValue << "<" << std::endl;
		throw SingleServerConfig::InvalidKeyException();
		break;
	}
	}
	#ifdef SHOW_LOG_2
		if (value.length() > 0)
			std::cout << "key >" << BLUE << key << RESET << "< value >" << YELLOW << value << RESET << "< added successfully to ConfigStruct" << std::endl;
	#endif
}

unsigned short SingleServerConfig::_checkListen(std::string value)
{
	size_t buffer = this->_strToSizeT(value);
	if (buffer > USHRT_MAX)
	{
		std::cout << RED << buffer << RESET << std::endl;
		throw SingleServerConfig::InvalidPortException();
	}
	unsigned short port = buffer;
	return (port);
	// std::cout << BLUE << "in _checkListen: >" << YELLOW << value << BLUE << "<" RESET << std::endl;
}

// void SingleServerConfig::_handleCgi(std::string line)
// {
// 	(void)line;
// 	// std::cout << BLUE << "in _handleCgi: >" << YELLOW << line << BLUE << "<" RESET << std::endl;
// }

enum
{
	location_root,
	location_method,
	location_index,
	location_auto_index
};

std::string locationVariables[]=
{
	"root",
	"method",
	"index_page",
	"autoindex"
};

LocationStruct SingleServerConfig::_initLocationStruct()
{
	LocationStruct locationStruct;
	locationStruct.isDir = true;
	locationStruct.autoIndex = false;
	locationStruct.getAllowed = false;
	locationStruct.postAllowed = false;
	locationStruct.deleteAllowed = false;
	locationStruct.root = "";
	locationStruct.indexPage = "";

	return (locationStruct);
}

LocationStruct SingleServerConfig::_fillLocationStruct(std::string block)
{
	// #ifdef SHOW_LOG_2
		std::cout << ">" << YELLOW << block << RESET << "< landed in _fillLocationStruct" << std::endl << std::endl;
	// #endif

	LocationStruct locationStruct = this->_initLocationStruct();
	std::stringstream blockStream;
	blockStream << block;
	std::string keyValue;
	std::string key;
	std::string value;

	bool foundRoot = false;
	bool foundMethod = false;
	bool foundIndex = false;
	bool foundAutoIndex = false;

	while (blockStream.good())
	{
		std::getline(blockStream, keyValue);
		if (keyValue.length() <= 0)
			continue ;
		if (keyValue.find_first_of(WHITESPACE) == std::string::npos)
		{
			std::cout << RED << keyValue << RESET << std::endl;
			throw SingleServerConfig::InvalidKeyValueException();
		}
		key = keyValue.substr(0, keyValue.find_first_of(WHITESPACE));
		size_t foundKey = location_root;

		for (; foundKey < location_auto_index + 1; ++foundKey)
		{
			if (locationVariables[foundKey] == key)
				break ;
		}

		switch (foundKey)
		{
		case (location_root):
		{
			if (foundRoot == true)
				throw SingleServerConfig::DuplicateLocationRootException();
			if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
			{
				std::cout << RED << keyValue << std::endl;
				throw SingleServerConfig::InvalidWhitespaceException();
			}
			value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
			locationStruct.root = value;
			foundRoot = true;
			break ;
		}

		case (location_method):
		{
			if (keyValue.find_last_not_of(WHITESPACE) - keyValue.find_first_of(WHITESPACE) > 15)
			{
				std::cout << RED << keyValue << RESET << std::endl;
				throw SingleServerConfig::InvalidMethodValueException();
			}
			value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
			for (; value.length() > 0;)
			{
				if (value.find_first_of(WHITESPACE) < value.find_first_not_of(WHITESPACE))
				{
					std::cout << RED << keyValue << RESET << std::endl;
					throw SingleServerConfig::InvalidWhitespaceException();
				}
				// std::cout << " top >" << value << "<" << std::endl;
				std::string tempValue = value.substr(0, value.find_first_of(WHITESPACE));
				if (tempValue == "GET" && locationStruct.getAllowed == false)
					locationStruct.getAllowed = true;
				else if (tempValue == "POST" && locationStruct.postAllowed == false)
					locationStruct.postAllowed = true;
				else if (tempValue == "DELETE" && locationStruct.deleteAllowed == false)
					locationStruct.deleteAllowed = true;
				else
				{
					std::cout << RED << tempValue << RESET << std::endl;
					throw SingleServerConfig::InvalidMethodValueException();
				}
				if (value.find_first_of(WHITESPACE) != std::string::npos)
					value = value.substr(value.find_first_of(WHITESPACE) + 1);
				else
					value = "";
				// std::cout << "bottom >" << value << "<" << std::endl;
			}
			foundMethod = true;
			break ;
		}

		case (location_index):
		{
			if (foundIndex == true)
				throw SingleServerConfig::DuplicateLocationIndexException();
			if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
			{
				std::cout << RED << keyValue << std::endl;
				throw SingleServerConfig::InvalidWhitespaceException();
			}
			value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
			locationStruct.indexPage = value;
			foundIndex = true;
			break ;
		}

		case (location_auto_index):
		{
			if (foundAutoIndex == true)
				throw SingleServerConfig::DuplicateLocationAutoIndexException();
			if (keyValue.find_first_of(WHITESPACE) != keyValue.find_last_of(WHITESPACE))
			{
				std::cout << RED << keyValue << std::endl;
				throw SingleServerConfig::InvalidWhitespaceException();
			}
			value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
			if (value != "true" && value != "false")
			{
				std::cout << RED << keyValue << std::endl;
				throw SingleServerConfig::InvalidValueTypeException();
			}
			locationStruct.autoIndex = (value.compare("true") == 0);
			foundAutoIndex = true;
			break ;
		}

		default:
		{
			std::cout << RED << keyValue << RESET << std::endl;
			throw SingleServerConfig::InvalidKeyException();
			break ;
		}
		}
	}
	if (!foundRoot || !foundMethod)
	{
		if (!foundRoot)
			std::cout << RED << "No root found in location " << key << RESET << std::endl;
		else
			std::cout << RED << "No method found in location " << key << RESET << std::endl;

		throw SingleServerConfig::InvalidLocationException();
	}
	if (foundIndex == true && foundAutoIndex == true)
	{
		std::cout << RED << "location " << key << RESET << std::endl;
		throw SingleServerConfig::InvalidIndexCombinationException();
	}
	else if (!foundIndex && !foundAutoIndex)
	{
		std::cout << RED << "location " << key << RESET << std::endl;
		throw SingleServerConfig::MissingIndexException();
	}


	return (locationStruct);
}

std::string SingleServerConfig::_printLocationStruct(LocationStruct locationStruct)
{
	std::stringstream outStream;
	outStream << "\t{\n\t\troot " << locationStruct.root << std::endl \
	<< "\t\tmethod ";
	if (locationStruct.getAllowed)
		outStream << "GET ";
	if (locationStruct.postAllowed)
		outStream << "POST ";
	if (locationStruct.deleteAllowed)
		outStream << "DELETE";
	outStream << std::endl;
	if (locationStruct.autoIndex)
		outStream << "\t\tautoindex true" << std::endl;
	else
	{
		outStream << "\t\tindex " << locationStruct.indexPage << std::endl;
	}
	outStream << "\t}" << std::endl;

	return (outStream.str());
}

void SingleServerConfig::_handleLocation(std::string block)
{
	std::stringstream bufferStream;
	bufferStream << block;
	std::string line;
	std::getline(bufferStream, line);
	if (line.substr(0, line.find_first_of(WHITESPACE)) != "location")
	{
		std::cout << RED << line.substr(0, line.find_first_of(WHITESPACE)) << RESET << std::endl;
		throw SingleServerConfig::InvalidKeyException();
	}
	else
	{
		std::string key = line.substr(line.find_first_of(WHITESPACE) + 1);
		if (key.substr(key.find_first_of(WHITESPACE) + 1) != "{")
		{
			throw SingleServerConfig::InvalidLocationBlockException();
		}
		key = key.substr(0, key.find_first_of(WHITESPACE));

		if (this->_conf->location.count(key) == 1)
		{
			std::cout << RED << key << RESET << std::endl;
			throw SingleServerConfig::DuplicateLocationException();
		}
		bool openBrackets = true;
		std::stringstream blockStream;
		while (bufferStream.good() && openBrackets == true)
		{
			std::getline(bufferStream, line);
			if (line == "}")
				openBrackets = false;
			else if (openBrackets == false)
			{
				std::cout << RED << line << RESET << std::endl;
				throw SingleServerConfig::InvalidLocationBlockException();
			}
			else
				blockStream << line << std::endl;
		}

		LocationStruct locationStruct = this->_fillLocationStruct(blockStream.str());
		if (key.substr(key.length() - 1) == "/" && key.find("*") == std::string::npos) // find out if it is a dir or not
			locationStruct.isDir = true;
		else if (key.substr(0, 2) == "*." && key.find("/") == std::string::npos)
			locationStruct.isDir = false;
		else
		{
			std::cout << RED << ">" << key << "< is not a allowed pattern" << RESET << std::endl;
			throw SingleServerConfig::InvalidLocationException(); // if the location key is not recognized
		}

		#ifdef SHOW_LOG_2
			std::cout << "key >" << BLUE << key << RESET << "< value\n" << YELLOW << this->_printLocationStruct(locationStruct) << RESET << "< added successfully to ConfigStruct" << std::endl;
		#endif
		this->_conf->location.insert(std::make_pair<std::string, LocationStruct>(key, locationStruct));
	}
}

std::string validErrorCodes[] =
{
	"100",
	"101",
	"102",
	"103",
	"200",
	"201",
	"202",
	"203",
	"204",
	"205",
	"206",
	"207",
	"208",
	"226",
	"300",
	"301",
	"302",
	"303",
	"304",
	"305",
	"306",
	"307",
	"308",
	"400",
	"401",
	"402",
	"403",
	"404",
	"405",
	"406",
	"407",
	"408",
	"409",
	"410",
	"411",
	"412",
	"413",
	"414",
	"415",
	"416",
	"417",
	"418",
	"421",
	"422",
	"423",
	"424",
	"425",
	"426",
	"428",
	"429",
	"431",
	"451",
	"500",
	"501",
	"502",
	"503",
	"504",
	"505",
	"506",
	"507",
	"508",
	"510",
	"511",
	""
};

static bool _isValidErrorCode(std::string errorCode)
{
	// std::cout << errorCode << " handed to check if it a valid errorCode" << std::endl;
	for (size_t i = 0; i < 64; ++i)
	{
		if (validErrorCodes[i] == errorCode)
			return (true);
	}
	return (false);
}

void SingleServerConfig::_handleErrorPage(std::string line)
{
	if (line.substr(0, line.find_first_of(WHITESPACE)) != "error_page")
	{
		std::cout << RED << line.substr(0, line.find_first_of(WHITESPACE)) << RESET << std::endl;
		throw SingleServerConfig::InvalidKeyException();
	}
	else
	{
		std::string key = line.substr(line.find_first_of(WHITESPACE) + 1);
		std::string value = key.substr(key.find_first_of(WHITESPACE) + 1);
		// std::cout << GREEN << ">" << value << "<" << RESET << std::endl;
		key = key.substr(0, key.find_first_of(WHITESPACE));
		if (value.find_first_of(WHITESPACE) != std::string::npos)
		{
			std::cout << RED << line << RESET << std::endl;
			throw SingleServerConfig::InvalidWhitespaceException();
		}
		else if (key.length() != 3 || _isValidErrorCode(key) == false)
		{
			std::cout << RED << key << RESET << std::endl;
			throw SingleServerConfig::NotAnErrorCodeException();
		}
		else if (this->_conf->errorPage.count(key) == 1)
		{
			std::cout << RED << key << RESET << std::endl;
			throw SingleServerConfig::DuplicateErrorPageException();
		}
		else
		{
			#ifdef SHOW_LOG_2
				std::cout << "key >" << BLUE << key << RESET << "< value >" << YELLOW << value << RESET << "< added successfully to ConfigStruct" << std::endl;
			#endif
			this->_conf->errorPage.insert(std::make_pair<std::string, std::string>(key, value));
		}
	}
}

size_t SingleServerConfig::_strToSizeT(std::string str)
{
	// std::cout << "str send to _strToSizeT >" << str << "<" << std::endl;

	size_t out = 0;
	std::stringstream buffer;
	#ifdef __APPLE__
		buffer << SIZE_T_MAX;
	#else
		buffer << "18446744073709551615";
	#endif
	std::string sizeTMax = buffer.str();
	if (str.find("-") != std::string::npos && str.find_first_of(DECIMAL) != std::string::npos && str.find("-") == str.find_first_of(DECIMAL) - 1)
	{
		std::cout << str << std::endl;
		throw SingleServerConfig::NegativeDecimalsNotAllowedException();
	}
	else if (str.find_first_of(DECIMAL) != std::string::npos)
	{
		std::string number = str.substr(str.find_first_of(DECIMAL));
		if (number.find_first_not_of(WHITESPACE) != std::string::npos)
			number = number.substr(0, number.find_first_not_of(DECIMAL));
		if (str.length() >= sizeTMax.length() && sizeTMax.compare(number) > 0)
		{
			std::cout << RED << ">" << number << RESET << std::endl;
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
	return ("↑↑↑ invalid port in .conf file found");
}

const char* SingleServerConfig::InvalidKeyException::what(void) const throw()
{
	return ("↑↑↑ invalid key for the .conf file found");
}

const char* SingleServerConfig::SizeTOverflowException::what(void) const throw()
{
	return ("↑↑↑ this number is too big, stay <=18446744073709551615");
}

const char* SingleServerConfig::NegativeDecimalsNotAllowedException::what(void) const throw()
{
	return ("↑↑↑ no negative decimals allowed as input");
}

const char* SingleServerConfig::InvalidPortException::what(void) const throw()
{
	return ("↑↑↑ this is an invalid value for a port");
}

const char* SingleServerConfig::InvalidWhitespaceException::what(void) const throw()
{
	return ("↑↑↑ invalid whitespace found");
}

const char* SingleServerConfig::InvalidValueTypeException::what(void) const throw()
{
	return ("↑↑↑ this does not fit the required argument type");
}

const char* SingleServerConfig::NotAnErrorCodeException::what(void) const throw()
{
	return ("↑↑↑ this is not a valid error code");
}

const char* SingleServerConfig::DuplicateErrorPageException::what(void) const throw()
{
	return ("↑↑↑ this error code was already defined before, only once per server allowed");
}

const char* SingleServerConfig::DefaultNotAllowedException::what(void) const throw()
{
	return ("server name <default> not allowed");
}

const char* SingleServerConfig::NoValueFoundException::what(void) const throw()
{
	return ("↑↑↑ please provide a value for this key");
}

const char* SingleServerConfig::InvalidLocationBlockException::what(void) const throw()
{
	std::cout << RED << "wrong syntax, has to be like >\n" << BLUE << "location *.filetype {\n\troot path/to/the/real/location\n\tmethod ALLOWED_METHOD/S\n\tautoindex off\n}\n" << RED << "< or >" << BLUE << "location /path/to/location/ {\n\troot path/to/the/real/location\n\tmethod ALLOWED_METHOD/S\n\tindex index_to_show\n\tautoindex off\n}\n" << std::endl;
	return ("↑↑↑ invalid location-block found, please provide like shown");
}

const char* SingleServerConfig::DuplicateLocationException::what(void) const throw()
{
	return ("↑↑↑ only one location-block with same key allowed");
}

const char* SingleServerConfig::InvalidKeyValueException::what(void) const throw()
{
	return ("↑↑↑ this key-value pair is invalid");
}

const char* SingleServerConfig::InvalidMethodValueException::what(void) const throw()
{
	return ("↑↑↑ this method is not recognized as a valid method or was already declared");
}

const char* SingleServerConfig::InvalidLocationException::what(void) const throw()
{
	return ("↑↑↑ caused the location-block to be invalid");
}

const char* SingleServerConfig::DuplicateLocationRootException::what(void) const throw()
{
	return ("double use of root key is not allowed");
}

const char* SingleServerConfig::DuplicateLocationIndexException::what(void) const throw()
{
	return ("double use of index_page key is not allowed");
}

const char* SingleServerConfig::DuplicateLocationAutoIndexException::what(void) const throw()
{
	return ("double use of autoindex key is not allowed");
}

const char* SingleServerConfig::InvalidIndexCombinationException::what(void) const throw()
{
	return ("↑↑↑ autoindex can not be true if a index_page is defined, vice versa");
}

const char* SingleServerConfig::MissingIndexException::what(void) const throw()
{
	return ("↑↑↑ autoindex can not be false if no index_page defined");
}
