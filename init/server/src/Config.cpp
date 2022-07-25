#include "Config.hpp"

// Private Methods
void Config::_openConfigFile()
{
	this->_configFile.open(this->_configPath.c_str());
	if (!this->_configFile.is_open())
	{
		// std::cerr << RED << "Error when opening " << this->_configPath << RESET << std::endl;
		// << "check for spelling errors in the name and check for read-rights of the file" << std::endl;
		throw Config::FileOpenException();
	}
}

void Config::_checkBrackets(std::string all)
{
	// std::cout << ">" << GREEN << all << RESET << "<" << std::endl;

	std::stringstream streamBuffer;
	streamBuffer << all;
	std::stringstream serverStream;
	bool openServer = false;
	bool openLocation = false;
	std::string buffer;
	while (streamBuffer.good())
	{
		std::getline(streamBuffer, buffer);
		if (buffer.length() == 0)
			continue ;
		serverStream << buffer << std::endl; // fill stringstream for the SingleServerConfig
		// std::cerr << ">" << YELLOW << buffer << RESET << "< got written into serverStream" << std::endl;
		if (buffer.find("server {") != std::string::npos && buffer.find_first_of("#;") > buffer.find("server {"))
		{
			// std::cout << BLUE << "opened server brackets with >" << buffer << "<" << RESET << std::endl;
			if (buffer.substr(buffer.find("server {") + 8).find_first_not_of(WHITESPACE) < buffer.substr(buffer.find("server {") + 8).find_first_of("#;") || (buffer.find_first_not_of(WHITESPACE) != buffer.find("server {")))
			{
				// std::cerr << "1>" << RED << buffer << RESET << "<" << std::endl;
				throw Config::WrongConfigSyntaxException();
			}
			else if (openServer == true)
			{
				// std::cerr << "1>" << RED << buffer << RESET << "<" << std::endl;
				throw Config::ServerInsideServerException();
			}
			else
				openServer = true;
		}
		else if (buffer.find("location ") != std::string::npos && buffer.find_first_of("#;") > buffer.find("location "))
		{
			// std::cout << BLUE << "opened location brackets with >" << buffer << "<" << RESET << std::endl;
			if (buffer.find(" {") == std::string::npos || buffer.find(" {") > buffer.find_first_of("#;"))
			{
				std::cout << RED << buffer << std::endl;
				throw Config::WrongListenBlockException();
			}
			else if (openServer == false || openLocation == true)
			{
				std::cout << RED << buffer << std::endl;
				throw Config::WrongListenBlockException();
			}
			else
				openLocation = true;
		}
		else if (buffer.find("}") != std::string::npos && buffer.find_first_of("#;") > buffer.find("}"))
		{
			// std::cerr << GREEN << "found this to close a bracket >" << buffer << "<" << RESET << std::endl;
			if (buffer.substr(buffer.find("}") + 1).find_first_not_of(WHITESPACE) < buffer.substr(buffer.find("}") + 1).find_first_of("#;") || buffer.find_first_not_of(WHITESPACE) != buffer.find("}"))
			{
				// std::cerr << "3>" << RED << buffer << RESET << "<" << std::endl;
				throw Config::WrongConfigSyntaxException();
			}
			else if (openLocation)
			{
				// std::cout << YELLOW << "closed location brackets with >" << buffer << "<" << RESET << std::endl;
				openLocation = false;
			}
			else
			{
				// std::cout << YELLOW << "closed server brackets with >" << buffer << "<" << RESET << std::endl;
				openServer = false;
				this->_parseServerBlock(serverStream.str());
				serverStream.clear();
				serverStream.str(std::string());
			}
		}
	}
	if (openLocation || openServer)
	{
		throw Config::InvalidBracketsException();
	}
	else if (buffer.length() > 0 && (buffer.find_first_of("#;") == std::string::npos || buffer.find_first_not_of(WHITESPACE) < buffer.find_first_of("#;")))
	{
		std::cout << BLUE << ">" << buffer << "<" << RESET << std::endl;
		throw Config::ContentOutsideServerBlockException();
	}
}

void Config::_parseServerBlock(std::string serverBlock)
{
	std::string server;
	std::string buffer;
	std::stringstream serverStream;
	serverStream << serverBlock;
	bool serverFound = false;

	// std::cout << "this was passed into the _parseServerBlock\n>" << serverBlock << "<" << std::endl;

	while (serverStream.good())
	{
		buffer.clear();
		std::getline(serverStream, buffer);
		if (buffer.length() == 0)
			continue ;
		// std::cerr << YELLOW << "before: >" << buffer << "<" << RESET << std::endl;
		size_t start = buffer.find_first_not_of(WHITESPACE); // clear out all the leading whitespaces
		if (start == std::string::npos)
			continue ;
		size_t end = buffer.find_first_of("#;"); // clear out all the comments and trailing whitespaces
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
			// std::cout << YELLOW << "server { found: >" << buffer << "<" << RESET << std::endl;
		}
		else if (serverFound == true && buffer.find("server {") != std::string::npos) // with new_ implementation not needed, check though !!!!!!!
		{
			// std::cerr << "2>" << RED << buffer << RESET << "<" << std::endl;
			throw Config::ServerInsideServerException();
		}
		if (serverFound == false && buffer.length() > 0) // might never be triggered, check this!!!!!!
		{
			throw Config::InvalidCharException();
		}
		if (buffer.length() > 0)
		{
			server.append(buffer);
			server.append("\n");
		}
	}
	if (server.length() == 0)
		throw Config::NoServerFoundException();
	this->_createConfigStruct(server);
}

void Config::_createConfigStruct(std::string server)
{
	static size_t structsCreated;
	// std::cerr << "This was passed into _createConfigStruct:\n>" << server << "<" << std::endl;

	if (server.find("server_name") == std::string::npos)
		throw Config::NoServerNameException();
	std::string serverName = server.substr(server.find("server_name"));
	serverName = serverName.substr(serverName.find_first_of(" ") + 1);
	serverName = serverName.substr(0, serverName.find_first_of("\n"));
	// std::cout << BLUE << "serverName: >" << RESET << serverName << BLUE << "<" << RESET << std::endl;
	if (this->_cluster.count(serverName) == 1)// check for duplicate serverName !!!!!!!!!!!!!!!
	{
		std::cerr << RED << serverName << std::endl;
		throw Config::DuplicateServerNameException();
	}
	ConfigStruct confStruct = this->_initConfigStruct();
	// confStruct = new ConfigStruct();
	// SingleServerConfig *SingleServerObject = new SingleServerConfig(server, confStruct);
	SingleServerConfig temp(server, &confStruct); // check the confStruct for correct values!!!!!!!!!!
	this->_cluster.insert(std::make_pair<std::string, ConfigStruct>(serverName, confStruct));
	++structsCreated;
	if (structsCreated == 1)
	{
		serverName = "default";
		this->_cluster.insert(std::make_pair<std::string, ConfigStruct>(serverName, confStruct));
	}
	// delete SingleServerObject;
	// std::cout << GREEN << "size: " << this->_cluster.size() << RESET << std::endl;
	// std::cout << GREEN << "added server " << serverName << " to cluster" << RESET << std::endl;
}

ConfigStruct Config::_initConfigStruct() // think about using defines in the Base.hpp or Config.hpp to set the default value so they are easy to change!!!!!
{
	ConfigStruct confStruct;
	confStruct.serverName = "";
	confStruct.listen = std::map<std::string, unsigned short>();
	confStruct.root = "";
	confStruct.autoIndex = false;
	confStruct.indexPage = "index.html";
	confStruct.chunkedTransfer = false; // maybe not needed because it is always chunked ????????
	confStruct.clientBodyBufferSize = 64000;
	confStruct.clientMaxBodySize = 256000;
	// confStruct.cgi = std::vector<std::string>();
	confStruct.cgiBin = "cgi-bin";
	confStruct.location = std::map<std::string, LocationStruct>();
	confStruct.errorPage = std::map<std::string, std::string>();
	confStruct.showLog = false;

	return (confStruct);
}

void Config::_freeConfigStruct()
{
	// check if freeing is needed!!!!!!!!!!!!!
	// this->_conf.listen.clear();
	// this->_conf.cgi.clear();
	// this->_conf.location.clear();
	// this->_conf.errorPage.clear();
}

void Config::_readConfigFile()
{
	std::stringstream streamBuffer;
	streamBuffer << this->_configFile.rdbuf();
	this->_configFile.close();
	std::string buffer = streamBuffer.str();
	this->_checkBrackets(buffer);
}

// Constructor
Config::Config()
{
	std::cout << GREEN << "Config Default Constructor called " << RESET << std::endl;
}

Config::Config(std::string configPath)
{
	std::cout << GREEN << "Config Constructor called" << RESET << std::endl;
	this->start(configPath);
}

// Deconstructor
Config::~Config()
{
	std::map<std::string, ConfigStruct>::iterator it;
	for (it = this->_cluster.begin(); it != this->_cluster.end(); ++it)
	{
		this->applyConfig(it->first);
		this->_freeConfigStruct();
	}
	// std ::cout << _cluster.size() << std::endl;
	this->_cluster.clear();
	// std ::cout << _cluster.size() << std::endl;
	std::cout << RED << "Config Deconstructor called"<< RESET << std::endl;
}

// Public Methods
void Config::start(std::string configPath)
{
	this->setConfigPath(configPath);
	this->_openConfigFile();
	// std::cout << GREEN << "allocating cluster map now" << std::endl;
	// this->_cluster = std::map<std::string, SingleServerConfig>();
	// std::cout << _cluster << RESET << std::endl;
	this->_readConfigFile();
	// std::cout << "finished start function of config object" << std::endl;
}

void Config::printCluster()
{
	std::map<std::string, ConfigStruct>::const_iterator it = this->_cluster.begin();
	for (; it != this->_cluster.end(); ++it)
	{
		this->applyConfig(it->first);
		std::cout << GREEN << it->first << " {" << RESET << std::endl << \
		"\tlisten\n" << this->strGetListen() << \
		"\troot " << this->strGetRoot() << std::endl << \
		"\tserver_name " << this->strGetServerName() << std::endl << \
		"\tautoindex " << this->strGetAutoIndex() << std::endl << \
		"\tindex_page " << this->strGetIndexPage() << std::endl << \
		"\tchunked_transfer " << this->strGetChunkedTransfer() << std::endl << \
		"\tclient_body_buffer_size " << this->strGetClientBodyBufferSize() << std::endl << \
		"\tclient_max_body_size " << this->strGetClientMaxBodySize() << std::endl << \
		/*"\tcgi " << a->strGetCgi() << std::endl << \*/
		"\tcgi_bin " << this->strGetCgiBin() << std::endl << \
		"\tlocation " << this->strGetLocation() << std::endl << \
		"\terror_page\n" << this->strGetErrorPage() << \
		"\tlog_level " << this->strGetShowLog() << std::endl << \
		 GREEN << "}" << RESET << std::endl << std::endl;
	}
}

// Getter
ConfigStruct Config::getConfigStruct(std::string serverName)
{
	std::string defaultConfig = "default";
	if (this->applyConfig(serverName) == true)
		return (this->_conf);
	else
	{
		std::cout << std::endl << RED << BOLD << "!!!!! DEFAULT STRUCT IS NOW BEEING USED !!!!!" << RESET << std::endl << std::endl;
		this->applyConfig(defaultConfig);
	}
	return (this->_conf);
}

const std::map<std::string, unsigned short> Config::getListen() const
{
	return (this->_conf.listen);
}

const std::string Config::getRoot() const
{
	return (this->_conf.root);
}

const std::string Config::getServerName() const
{
	return (this->_conf.serverName);
}

bool Config::getAutoIndex() const
{
	return (this->_conf.autoIndex);
}

const std::string Config::getIndexPage() const
{
	return (this->_conf.indexPage);
}

bool Config::getChunkedTransfer() const
{
	return (this->_conf.chunkedTransfer);
}

size_t Config::getClientBodyBufferSize() const
{
	return (this->_conf.clientBodyBufferSize);
}

size_t Config::getClientMaxBodySize() const
{
	return (this->_conf.clientMaxBodySize);
}

// const std::vector<std::string> Config::getCgi() const
// {
// 	return (this->_conf.cgi);
// }

const std::string Config::getCgiBin() const
{
	return (this->_conf.cgiBin);
}

const std::map<std::string, LocationStruct> Config::getLocation() const
{
	return (this->_conf.location);
}

const std::map<std::string, std::string> Config::getErrorPage() const
{
	return (this->_conf.errorPage);
}

bool Config::getShowLog() const
{
	return (this->_conf.showLog);
}

// Getters for printing
const std::string Config::strGetListen() const
{
	std::stringstream print;
	std::map<std::string, unsigned short>::const_iterator it = this->_conf.listen.begin();
	for (; it != this->_conf.listen.end(); ++it)
		print << "\t\t" << it->second << std::endl;
	return (print.str());
}

const std::string Config::strGetRoot() const
{
	std::string print = getRoot();
	return (print);
}

const std::string Config::strGetServerName() const
{
	std::string print = getServerName();
	return (print);
}

const std::string Config::strGetAutoIndex() const
{
	std::string print;
	if (getAutoIndex() == true)
		print = "true";
	else
		print = "false";
	return (print);
}

const std::string Config::strGetIndexPage() const
{
	std::string print = getIndexPage();
	return (print);
}

const std::string Config::strGetChunkedTransfer() const
{
	std::string print;
	if (getChunkedTransfer() == true)
		print = "true";
	else
		print = "false";
	return (print);
}

const std::string Config::strGetClientBodyBufferSize() const
{
	std::stringstream print;
	print << getClientBodyBufferSize();
	return (print.str());
}

const std::string Config::strGetClientMaxBodySize() const
{
	std::stringstream print;
	print << getClientMaxBodySize();
	return (print.str());
}

// const std::string Config::strGetCgi() const
// {
// 	std::string print = "";
// 	size_t size = this->_conf.cgi.size();
// 	for (size_t i = 0; i < size; ++i)
// 	{
// 		print.append(this->_conf.cgi[i]);
// 		print.append(" ");
// 	}
// 	return (print);
// }

const std::string Config::strGetCgiBin() const
{
	std::string print = getCgiBin();
	return (print);
}

const std::string Config::strGetLocation() const
{
	std::stringstream print;
	std::map<std::string, LocationStruct>::const_iterator it = this->_conf.location.begin();
	for (; it != this->_conf.location.end(); ++it)
		print << "\t\t" << it->first << " " << "placeholder for LocationStruct" << std::endl;
	print << "/placeholer\n\t\t\troot place/hold/placeholder\n\t\t\tmethod GET POST DELETE\n\t\t\tautoindex true";
	return (print.str());
}

const std::string Config::strGetErrorPage() const
{
	std::stringstream print;
	std::map<std::string, std::string>::const_iterator it = this->_conf.errorPage.begin();
	for (; it != this->_conf.errorPage.end(); ++it)
		print << "\t\t" << it->first << " " << it->second << std::endl;
	return (print.str());
}

const std::string Config::strGetShowLog() const
{
	std::string print;

	if (this->getShowLog() == true)
		print = "true";
	else
		print = "false";
	return (print);
}

// Setter
void Config::setConfigPath(std::string configPath)
{
	// some error checking
	this->_configPath = configPath;
}

bool Config::applyConfig(std::string serverName)
{
	if (this->_cluster.count(serverName) == 1)
	{
		this->_conf = this->_cluster[serverName];
		return (true);
	}
	else
		return (false); // maybe throw an exception
}

// Exceptions
// brackets can not be opened and closed on the same line
// no content after the opening bracket on same line, comments are ok
// no content on closing bracket line, cmments are ok
// incomplete brackets
const char* Config::InvalidBracketsException::what(void) const throw()
{
	return ("Invalid brackets in .conf file");
}

const char* Config::FileOpenException::what(void) const throw()
{
	return ("Failed to read from .conf file, check file existance and readrights");
}

const char* Config::ServerInsideServerException::what(void) const throw()
{
	return ("Wrong Syntax in .conf file, server-block inside server-block found");
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

const char* Config::DuplicateServerNameException::what(void) const throw()
{
	return ("↑↑↑ duplicate server_name found in the .conf file found, see above");
}

const char* Config::WrongListenBlockException::what(void) const throw()
{
	return ("↑↑↑ wrong listen-block found inside .conf file, see above");
}

const char* Config::ContentOutsideServerBlockException::what(void) const throw()
{
	return ("non-whitespace and non-commented content is forbidden outside server-block");
}

const char* Config::NoServerFoundException::what(void) const throw()
{
	return ("please provide at least one server-block in .conf file");
}

// Ostream overload
std::ostream	&operator<<(std::ostream &o, Config *a) // set the correct struct by giving the appropriate serverName to the setPrintFunction
{
	o << a->strGetServerName() << " {" << std::endl << \
	"\tlisten\n" << a->strGetListen() << \
	"\troot " << a->strGetRoot() << std::endl << \
	"\tserver_name " << a->strGetServerName() << std::endl << \
	"\tautoindex " << a->strGetAutoIndex() << std::endl << \
	"\tindex_page " << a->strGetIndexPage() << std::endl << \
	"\tchunked_transfer " << a->strGetChunkedTransfer() << std::endl << \
	"\tclient_body_buffer_size " << a->strGetClientBodyBufferSize() << std::endl << \
	"\tclient_max_body_size " << a->strGetClientMaxBodySize() << std::endl << \
	/*"\tcgi " << a->strGetCgi() << std::endl << \*/
	"\tcgi_bin " << a->strGetCgiBin() << std::endl << \
	"\tlocation " << a->strGetLocation() << std::endl << \
	"\terror_page\n" << a->strGetErrorPage() << \
	"\tlog_level " << a->strGetShowLog() << std::endl << \
	"}" << std::endl;
	return (o);
}
