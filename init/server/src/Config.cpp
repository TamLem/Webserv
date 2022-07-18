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
		while (streamBuffer.good()) // fill SingleServerConfig
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
			// std::cerr << YELLOW << "before: >" << buffer << "<" << RESET << std::endl;
			size_t start = buffer.find_first_not_of(WHITESPACE);
			if (start == std::string::npos)
				continue ;
			size_t end = buffer.find_first_of('#');
			if (end == std::string::npos)
			{
				end = buffer.find_last_not_of(WHITESPACE);
				buffer = buffer.substr(start, (end - start + 1));
				// std::cerr << BLUE << "if: >" << buffer << "<" << RESET << std::endl;
			}
			else
			{
				--end;
				buffer = buffer.substr(start, (end - start + 1));
				end = buffer.find_last_not_of(WHITESPACE);
				buffer = buffer.substr(0, end + 1);
				// std::cerr << BLUE << "else: >" << buffer << "<" << RESET << std::endl;
			}
			if (serverFound == false && buffer.find("server {") != std::string::npos)
			{
				serverFound = true;
				std::cout << YELLOW << "server { found: >" << buffer << "<" << RESET << std::endl;
			}
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
		// std::cerr << "serverStream not good anymore" << std::endl;
		// std::cerr << RED << "server: >" << server << "< with length of: " << server.length() << RESET << std::endl;
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
		// check for duplicate serverName !!!!!!!!!!!!!!!
		// SingleServerConfig *serverObject = new SingleServerConfig(server); // needs to be allocated, maybe not!!!???????
		this->_cluster->insert(std::make_pair<std::string, SingleServerConfig>(serverName, SingleServerConfig(server)));
		std::cout << RED << "added server " << serverName << " to cluster" << RESET << std::endl;
		server.clear();
		serverName.clear();
	}
}

// Constructor
Config::Config()
{
	std::cout << "Config Default Constructor called" << std::endl;
}

Config::Config(std::string configPath)
{
	std::cout << "Config Constructor called" << std::endl;
	this->start(configPath);
}

// Deconstructor
Config::~Config()
{
	this->_cluster->clear();
	delete this->_cluster;
	std::cout << "Config Deconstructor called" << std::endl;
}

// Public Methods
void Config::start(std::string configPath)
{
	this->setConfigPath(configPath);
	this->_openConfigFile();
	// std::cout << GREEN << "allocating map now" << std::endl;
	this->_cluster = new std::map<std::string, SingleServerConfig>;
	// std::cout << GREEN << _cluster << RESET << std::endl;
	this->_readConfigFile();
}

// Getter
const std::string Config::getConfigPath() const
{
	return (this->_configPath);
}

std::map<std::string, SingleServerConfig> Config::getCluster() const
{
	// std::cout << GREEN << this->_cluster << RESET << std::endl;
	return (*this->_cluster);
}

// Setter
void Config::setConfigPath(std::string configPath)
{
	// some error checking
	this->_configPath = configPath;
}

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
