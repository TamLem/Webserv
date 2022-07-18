#include "SingleServerConfig.hpp"

#include <stdlib.h> // forbidden atoi is connected to this !!!!!!!!

// Constructors
SingleServerConfig::SingleServerConfig()
{
	std::cout << GREEN << "SingleServerConfig default constructor called: " << this << RESET << std::endl;
}
SingleServerConfig::SingleServerConfig(std::string config): _listen(), _root(), _serverName(), _autoIndex(false), _chunkedTransfer(false), _clientBodyBufferSize(16000), _clientMaxBodySize(64000), _cgi(), _cgiBin(), _location(), _errorPage(), _logLevel(0)
{
	std::cout << GREEN << "SingleServerConfig constructor called: " << this << RESET << std::endl;
	// std::cout << "This content reached the server:\n>" << config << "<" << std::endl;
	this->_setVariables(config);
}

// Deconstructors
SingleServerConfig::~SingleServerConfig()
{
	std::cout << RED << "SingleServerConfig deconstructor for " << this->_serverName << " called: " << this << RESET << std::endl;
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
		// std::cout << BLUE << buffer << "<-- reached the evaluateKeyValue function" << RESET << std::endl;
		this->_evaluateKeyValue(buffer);
		buffer.clear();
	}
	std::cout << "done" << std::endl;
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
	case (listen):
	{
		this->_listen.push_back(value);
		break ;
	}

	case (root):
	{
		this->_root = value;
		break ;
	}

	case (server_name):
	{
		this->_serverName = value;
		break;
	}

	case (autoindex):
	{
		this->_autoIndex = (value.compare("true") == 0);
		break ;
	}

	case (index_page):
	{
		this->_indexPage = value;
		break ;
	}

	case (chunked_transfer):
	{
		this->_chunkedTransfer = (value.compare("true") == 0);
		break ;
	}

	case (client_body_buffer_size):
	{
		this->_clientBodyBufferSize = atoi(value.c_str()); // check if forbidden!!!!!!!!
		break ;
	}

	case (client_max_body_size):
	{
		this->_clientMaxBodySize = atoi(value.c_str()); //check if forbidden !!!!!!!!!!!
		break ;
	}

	case (cgi):
	{
		this->_cgi.push_back(value); // this value needs to be checked again!!!!!!! it is wrong
		break ;
	}

	case (cgi_bin):
	{
		this->_cgiBin = value;
		break ;
	}

	case (location):
	{
		std::string first_arg = "first_arg";
		this->_location.insert(std::make_pair<std::string, std::string>(first_arg, value)); // this is also still wrong
		break ;
	}

	case (error_page):
	{
		this->_errorPage.push_back(value); // still wrong!!!!!!!!!!
		break ;
	}

	case (log_level):
	{
		this->_logLevel = atoi(value.c_str());
		break ;
	}

	// default:
	case (not_found):
	{
		std::cerr << RED << ">" << key << "<" << std::endl;
		throw SingleServerConfig::InvalidKeyException();
		break;
	}
	}
	std::cout << YELLOW << "key:\t>" << key << "<" << std::endl << BLUE << "value:\t>" << value << "<" << std::endl;
}

// Public Methods

// Getter
const SingleServerConfig *SingleServerConfig::getConfigAddress() const
{
	return (this);
}

const std::vector<std::string> SingleServerConfig::getListen() const
{
	return (this->_listen);
}

const std::string SingleServerConfig::getRoot() const
{
	return (this->_root);
}

const std::string SingleServerConfig::getServerName() const
{
	return (this->_serverName);
}

bool SingleServerConfig::getAutoIndex() const
{
	return (this->_autoIndex);
}

const std::string SingleServerConfig::getIndexPage() const
{
	return (this->_indexPage);
}

bool SingleServerConfig::getChunkedTransfer() const
{
	return (this->_chunkedTransfer);
}

size_t SingleServerConfig::getClientBodyBufferSize() const
{
	return (this->_clientBodyBufferSize);
}

size_t SingleServerConfig::getClientMaxBodySize() const
{
	return (this->_clientMaxBodySize);
}

const std::vector<std::string> SingleServerConfig::getCgi() const
{
	return (this->_cgi);
}

const std::string SingleServerConfig::getCgiBin() const
{
	return (this->_cgiBin);
}

const std::map<std::string, std::string> SingleServerConfig::getLocation() const
{
	return (this->_location);
}

const std::vector<std::string> SingleServerConfig::getErrorPage() const
{
	return (this->_errorPage);
}

size_t SingleServerConfig::getLogLevel() const
{
	return (this->_logLevel);
}

// Getters for printing
const std::string SingleServerConfig::strGetListen() const
{
	std::string print = "";
	size_t size = this->_listen.size();
	for (size_t i = 0; i < size; ++i)
	{
		print.append(this->_listen[i]);
		print.append(" ");
	}
	return (print);
}

const std::string SingleServerConfig::strGetRoot() const
{
	std::string print = getRoot();
	return (print);
}

const std::string SingleServerConfig::strGetServerName() const
{
	std::string print = getServerName();
	return (print);
}

const std::string SingleServerConfig::strGetAutoIndex() const
{
	std::string print;
	if (getAutoIndex() == true)
		print = "true";
	else
		print = "false";
	return (print);
}

const std::string SingleServerConfig::strGetIndexPage() const
{
	std::string print = getIndexPage();
	return (print);
}

const std::string SingleServerConfig::strGetChunkedTransfer() const
{
	std::string print;
	if (getChunkedTransfer() == true)
		print = "true";
	else
		print = "false";
	return (print);
}

const std::string SingleServerConfig::strGetClientBodyBufferSize() const
{
	std::stringstream print;
	print << getClientBodyBufferSize();
	return (print.str());
}

const std::string SingleServerConfig::strGetClientMaxBodySize() const
{
	std::stringstream print;
	print << getClientMaxBodySize();
	return (print.str());
}

const std::string SingleServerConfig::strGetCgi() const
{
	std::string print = "";
	size_t size = this->_cgi.size();
	for (size_t i = 0; i < size; ++i)
	{
		print.append(this->_cgi[i]);
		print.append(" ");
	}
	return (print);
}

const std::string SingleServerConfig::strGetCgiBin() const
{
	std::string print = getCgiBin();
	return (print);
}

const std::string SingleServerConfig::strGetLocation() const
{
	std::string print = "";
	std::map<std::string, std::string>::const_iterator it = this->_location.begin();
	for (; it != this->_location.end(); ++it)
	{
		if (print.length() != 0)
			print.append(" ");
		print.append(it->first);
		print.append(" ");
		print.append(it->second);
	}
	return (print);
}

const std::string SingleServerConfig::strGetErrorPage() const
{
	std::string print = "";
	size_t size = this->_errorPage.size();
	for (size_t i = 0; i < size; ++i)
	{
		print.append(this->_errorPage[i]);
		print.append(" ");
	}
	return (print);
}

const std::string SingleServerConfig::strGetLogLevel() const
{
	std::stringstream print;
	print << this->getLogLevel();
	return (print.str());
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
	return ("↑↑↑ invalid port in .conf file found, see above");
}

const char* SingleServerConfig::InvalidKeyException::what(void) const throw()
{
	return ("↑↑↑ invalid key for the .conf file found, see above");
}

// Ostream overload
std::ostream	&operator<<(std::ostream &o, SingleServerConfig a)
{
	o << a.strGetServerName() << " {" << std::endl << \
	"\tlisten " << a.strGetListen() << std::endl << \
	"\troot " << a.strGetRoot() << std::endl << \
	"\tserver_name " << a.strGetServerName() << std::endl << \
	"\tautoindex " << a.strGetAutoIndex() << std::endl << \
	"\tindex_page " << a.strGetIndexPage() << std::endl << \
	"\tchunked_transfer " << a.strGetChunkedTransfer() << std::endl << \
	"\tclient_body_buffer_size " << a.strGetClientBodyBufferSize() << std::endl << \
	"\tclient_max_body_size " << a.strGetClientMaxBodySize() << std::endl << \
	"\tcgi " << a.strGetCgi() << std::endl << \
	"\tcgi_bin " << a.strGetCgiBin() << std::endl << \
	"\tlocation " << a.strGetLocation() << std::endl << \
	"\terror_page " << a.strGetErrorPage() << std::endl << \
	"\tlog_level " << a.strGetLogLevel() << std::endl << \
	"}" << std::endl;
	return (o);
}
