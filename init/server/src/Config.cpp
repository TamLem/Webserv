#include "Config.hpp"
#include "Base.hpp"

// Private Methods
void Config::_openConfigFile()
{
	this->_configFile.open(this->getConfigPath());
	if (!this->_configFile.is_open())
	{
		// std::cerr << RED << "Error when opening " << this->getConfigPath() << RESET << std::endl \
		// << "\tcheck for spelling errors in the name and check for read-rights of the file" << std::endl;
		std::string error_msg = "check existance and file-rights of " + this->getConfigPath();
		#ifndef CAUSE
			#define CAUSE error_msg
		#endif
		throw Config::FileOpenException();
	}
}

enum configKeys
{
	listen,
	root,
	server_name,
	default_
};

std::string configCompare[] =
{
	"listen",
	"root",
	"server_name"
};

void Config::_setCause(std::string cause)
{
	this->_cause = cause;
}

void Config::_readConfigFile()
{
	std::stringstream streambuffer;
	streambuffer << this->_configFile.rdbuf();
	int nOpenBrackets = 0;
	std::string buffer;
	while (streambuffer.good()) // fill _cluster
	{
		std::stringstream server;
		while (streambuffer.good()) // fill SingleServer
		{
			std::getline(streambuffer, buffer);
			server << buffer << std::endl;
			if (buffer.find("{") != std::string::npos)
				++nOpenBrackets;
			if (buffer.find("}") != std::string::npos)
				--nOpenBrackets;
			if (buffer.find("}") != std::string::npos && nOpenBrackets == 0)
				break ;
		}

		// search for server_name
		std::string server_name = server.str();
		server.clear();
		server_name = server_name.substr(server_name.find("server_name"));
		server_name = server_name.substr(server_name.find_first_of(" ") + 1);
		server_name = server_name.substr(0, server_name.find_first_of("\n"));
		std::cout << BLUE << "server_name: >" << server_name << "<" << RESET << std::endl;
		this->_cluster->insert(std::make_pair<std::string, SingleServer>(server_name, SingleServer(server.str())));

		// while (server.good())
		// {
		// 	// fill SingleServer here
		// }
		buffer.clear();
	}
	if (nOpenBrackets != 0)
		throw Config::InvalidBracketsException();
	// std::map<size_t, std::string>::iterator token = this->_tokens.begin();
	// for (; token != this->_tokens.end(); token++)
	// {
		// if (token.key == "#")
		// 	continue ;
		// // std::remove(buffer.begin(), buffer.end(), '\t');
		// // std::remove(buffer.begin(), buffer.end(), '\n');
		// std::cerr << GREEN << "contentes of buffer: >" << buffer << "< has a length of: " << buffer.length() << RESET << std::endl;
		// size_t i = 0;
		// for (; i < default_; ++i)
		// {
		// 	if (buffer.compare(buffer.find_first_not_of(" \t\n"), buffer.find_first_of(" "), configCompare[i], 0, configCompare[i].length()) == 0)
		// 	{
		// 		std::cerr << BLUE << buffer << "<--->" << configCompare[i] << std::endl;
		// 		break ;
		// 	}
		// }
		// std::stringstream sbuffer;
		// switch (i)
		// {
		// 	case (listen):
		// 		std::cerr << RED << configCompare[listen] << " found" << RESET <<std::endl;
		// 		sbuffer << buffer.substr(buffer.find_first_of(' ') + 1);
		// 		size_t port;
		// 		sbuffer >> port;
		// 		this->setPort(port);
		// 		buffer.clear();
		// 		break ;
		// 	case (root):
		// 		std::cerr << RED << configCompare[root] << " found" << RESET <<std::endl;
		// 		buffer.clear();
		// 		break ;
		// 	case (server_name):
		// 		std::cerr << RED << configCompare[server_name] << " found" << RESET <<std::endl;
		// 		buffer.clear();
		// 		break ;
		// 	default:
		// 		std::cerr << RED << buffer << " not found in switch case" << RESET << std::endl;
		// 		buffer.clear();
		// 		break ;
		// }
		// std::cerr << YELLOW << "buffer after switch case: " << buffer << RESET << std::endl;
	// }
}

// Constructor
Config::Config()
{
}

Config::Config(std::string configPath)
{
	std::cout << "Config Default Constructor called" << std::endl;
	this->start(configPath);
}

// Deconstructor
Config::~Config()
{
	std::cout << "Config Deconstructor called" << std::endl;
	delete this->_cluster;
}

// Public Methods
void Config::start(std::string configPath)
{
	this->setConfigPath(configPath);
	this->_openConfigFile();
	this->_cluster = new std::map<std::string, SingleServer>;
	this->_readConfigFile();
}

// Getter
const std::string Config::getConfigPath() const
{
	return (this->_configPath);
}

const std::string Config::getCause() const
{
	return (this->_cause);
}

std::map<std::string, SingleServer> *Config::getCluster() const
{
	return (this->_cluster);
}

// Setter
void Config::setConfigPath(std::string configPath)
{
	// some error checking
	this->_configPath = configPath;
}

// Ostream overload
// std::ostream	&operator<<(std::ostream &o, Config *config)
// {
// 	o << "configPath: " << config->getConfigPath() << std::endl \
// 	<< "server:" << config->getServer() << std::endl \
// 	<< "rootPath: " << config->getRootPath() << std::endl;
// 	return (o);
// }

// Exceptions
const char* Config::InvalidBracketsException::what(void) const throw()
{
	return ("Invalid brackets in .conf-file");
}

const char* Config::FileOpenException::what(void) const throw()
{
	return ("Failed to read from .conf-file");
}
