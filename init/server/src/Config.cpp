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

// enum configKeys
// {
// 	listen,
// 	root,
// 	serverName,
// 	default_
// };

// std::string configCompare[] =
// {
// 	"listen",
// 	"root",
// 	"serverName"
// };

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
			if (buffer.length() == 0)
				continue ;
			serverStream << buffer << std::endl;
			// if (buffer.find("server {") != std::string::npos && buffer.substr(buffer.find("{") + 1).find_first_not_of(WHITESPACE) < buffer.substr(buffer.find("{") + 1).find("#"))
			// {
			// 	// std::cout << YELLOW << ">" << buffer.substr(buffer.find("{") + 1).find_first_not_of(WHITESPACE) << "<" << RESET << std::endl;
			// 	// std::cout << YELLOW << ">" << buffer.find("#") << RESET << std::endl;
			// 	throw Config::WrongConfigSyntaxException();
			// }
			if (buffer.find("{") != std::string::npos && buffer.find("#") > buffer.find("{"))
			{
				if (buffer.substr(buffer.find("{") + 1).find_first_not_of(WHITESPACE) < buffer.substr(buffer.find("{") + 1).find("#") || (buffer.find_first_not_of(WHITESPACE) != buffer.find("{") && buffer.find_first_not_of(WHITESPACE) != buffer.find("server {")))
					throw Config::WrongConfigSyntaxException();
				++nOpenBrackets;
			}
			else if (buffer.find("}") != std::string::npos && buffer.find("#") > buffer.find("}"))
			{
				if (buffer.substr(buffer.find("}") + 1).find_first_not_of(WHITESPACE) < buffer.substr(buffer.find("}") + 1).find("#") || buffer.find_first_not_of(WHITESPACE) != buffer.find("}"))
					throw Config::WrongConfigSyntaxException();
				--nOpenBrackets;
			}
			// std::cerr << RED << "buffer: >" << buffer << "<" << std::endl << RESET << "nOpenBrackets: " << nOpenBrackets << std::endl;
			if (buffer.find("}") != std::string::npos && buffer.find("#") > buffer.find("}") && nOpenBrackets == 0)
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
			std::cerr << YELLOW << "before: >" << buffer << "<" << RESET << std::endl;
			size_t start = buffer.find_first_not_of(WHITESPACE);
			if (start == std::string::npos)
				continue ;
			size_t end = buffer.find_first_of('#');
			if (end == std::string::npos)
			{
				end = buffer.find_last_not_of(WHITESPACE);
				buffer = buffer.substr(start, (end - start + 1));
				std::cerr << BLUE << "if: >" << buffer << "<" << RESET << std::endl;
			}
			else
			{
				--end;
				buffer = buffer.substr(start, (end - start + 1));
				end = buffer.find_last_not_of(WHITESPACE);
				buffer = buffer.substr(0, end + 1);
				std::cerr << BLUE << "else: >" << buffer << "<" << RESET << std::endl;
			}
			if (serverFound == false && buffer.find("server {") != std::string::npos)
				serverFound = true;
			else if (serverFound == true && buffer.find("server {") != std::string::npos)
				throw Config::ServerInsideServerException();
			if (serverFound == false && buffer.length() > 0)
			{
				throw Config::InvalidCharException();
			}
			if (buffer.length() > 0)
			{
				server.append(buffer);
				server.append("\n");
			}
		}
		std::cerr << "serverStream not good anymore" << std::endl;
		std::cerr << RED << "server: >" << server << "< with length of: " << server.length() << RESET << std::endl;
		if (nOpenBrackets != 0)
			throw Config::InvalidBracketsException();
		if (server.length() == 0)
			break ;// continue ;
		if (server.find("server_name") == std::string::npos)
			throw Config::NoServerNameException();
		std::string serverName = server.substr(server.find("server_name"));
		serverName = serverName.substr(serverName.find_first_of(" ") + 1);
		serverName = serverName.substr(0, serverName.find_first_of("\n"));
		std::cout << BLUE << "serverName: >" << RESET << serverName << BLUE << "<" << RESET << std::endl;
		SingleServer serverObject(server);
		this->_cluster->insert(std::make_pair<std::string, SingleServer>(serverName, serverObject));
		server.clear();
		serverName.clear();
	}
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
	std::cout << GREEN << _cluster << RESET << std::endl;
	this->_readConfigFile();
}

// Getter
const std::string Config::getConfigPath() const
{
	return (this->_configPath);
}

std::map<std::string, SingleServer> *Config::getCluster() const
{
	std::cout << GREEN << this->_cluster << RESET << std::endl;
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
// brackets can not be opened and closed on the same line
// no content after the opening bracket on same line, comments are ok
// no content on closing bracket line, cmments are ok
// incomplete brackets
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

const char* Config::InvalidCharException::what(void) const throw()
{
	return ("Invalid char found outside of a server-block");
}

const char* Config::NoServerNameException::what(void) const throw()
{
	return ("No server_name found inside the server-block");
}

const char* Config::WrongConfigSyntaxException::what(void) const throw()
{
	return ("No content on same line as server-block-start or end allowed");
}
