#include "Config.hpp"
#include "Base.hpp"

// Private Methods
void Config::_openConfigFile()
{
	this->_configFile.open(this->getConfigPath().c_str());
	if (!this->_configFile.is_open())
	{
		// std::cerr << RED << "Error when opening " << this->getConfigPath() << RESET << std::endl
		// << "\tcheck for spelling errors in the name and check for read-rights of the file" << std::endl;
		throw Config::FileOpenException();
	}
}

enum configKeys
{
	listen,
	root,
	serverName,
	default_
};

std::string configCompare[] =
{
	"listen",
	"root",
	"serverName"
};

void Config::_readConfigFile()
{
	std::stringstream streamBuffer;
	streamBuffer << this->_configFile.rdbuf();
	this->_configFile.close();
	int nOpenBrackets = 0;
	std::string buffer;
	while (streamBuffer.good()) // fill _cluster
	{
		std::stringstream serverStream;
		while (streamBuffer.good()) // fill SingleServer
		{
			std::getline(streamBuffer, buffer);
			serverStream << buffer << std::endl;
			if (buffer.find("{") != std::string::npos && buffer.find("#") == std::string::npos)
				++nOpenBrackets;
			if (buffer.find("}") != std::string::npos && buffer.find("#") == std::string::npos)
				--nOpenBrackets;
			if (buffer.find("}") != std::string::npos && buffer.find("#") == std::string::npos && nOpenBrackets == 0)
				break ;
		}

		// search for serverName
		// std::string serverName = serverStream.str();
		// serverStream.clear();
		// cut out comments from server
		std::string server;
		bool serverFound = false;
		while (serverStream.good()) // clear out all the comments, leading and trailing whitespaces
		{
			buffer.clear();
			std::getline(serverStream, buffer);
			if (buffer.length() == 0)
				continue ;
			size_t start = buffer.find_first_not_of(WHITESPACE);
			if (start == std::string::npos)
				continue ;
			size_t end = buffer.find_first_of('#');
			if (end == std::string::npos)
			{
				end = buffer.find_last_not_of(WHITESPACE);
				buffer = buffer.substr(start, (end - start + 1));
				std::cerr << BLUE << "if: >" << buffer << "<" << std::endl;
			}
			else
			{
				--end;
				buffer = buffer.substr(start, (end - start + 1));
				end = buffer.find_last_not_of(WHITESPACE);
				buffer = buffer.substr(0, end + 1);
				std::cerr << BLUE << "else: >" << buffer << "<" << std::endl;
			}
			if (serverFound == false && buffer.find("server {") != std::string::npos)
				serverFound = true;
			else if (serverFound == true && buffer.find("server {") != std::string::npos)
				throw Config::ServerInsideServerException();
			if (serverFound == false)
				continue ;
			if (buffer.length() > 0)
			{
				server.append(buffer);
				server.append(";");
			}
		}
		std::string serverName = server.substr(server.find("server_name"));
		serverName = serverName.substr(serverName.find_first_of(" ") + 1);
		serverName = serverName.substr(0, serverName.find_first_of(";"));
		std::cout << BLUE << "serverName: >" << serverName << "<" << RESET << std::endl;
		this->_cluster->insert(std::make_pair<std::string, SingleServer>(serverName, SingleServer(server)));
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
		// 	case (serverName):
		// 		std::cerr << RED << configCompare[serverName] << " found" << RESET <<std::endl;
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
// 	o << "configPath: " << config->getConfigPath() << std::endl
// 	<< "server:" << config->getServer() << std::endl
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

const char* Config::ServerInsideServerException::what(void) const throw()
{
	return ("Wrong Syntax in .conf-file, server-block inside server-block found");
}
