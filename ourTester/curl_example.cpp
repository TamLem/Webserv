#include <iostream>
#include <string>
#include <curl/curl.h>

// Colors and Printing
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RED "\033[31m"
#define BOLD "\033[1m"
#define UNDERLINED "\033[4m"

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

static long nothrow_stol(const std::string& string)
{
	long ret;
	try {
		ret = stol(string);
	}
	catch (std::exception &e)
	{
		return (-1);
	}
	return (ret);
}

static void curl_delete(const std::string& url, const std::string& expected)
{
	static int i = 1;
	long statuscode;
	CURL *curl;
	struct curl_slist *host = NULL;
	host = curl_slist_append(NULL, "webserv:80:127.0.0.1");
	curl_slist_append(host, "server1:6000:127.0.0.1");
	curl_slist_append(host, "server2:8080:127.0.0.1");
	CURLcode res;
	std::string readBuffer;
	curl = curl_easy_init();

	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_RESOLVE, host);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statuscode);
		res = curl_easy_perform(curl);
		if(res == CURLE_OK)
		{
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statuscode);
		}
		curl_easy_cleanup(curl);
		std::cout << YELLOW << "Test " << i << ": " << url << RESET << std::endl;
		if (readBuffer.compare(expected) == 0)
			std::cout << GREEN << "OK" << RESET << std::endl;
		else if (nothrow_stol(expected) == statuscode)
			std::cout << GREEN << "OK" << RESET << " status code: " << statuscode << std::endl;
		else if (readBuffer.find(expected) != std::string::npos)
			std::cout << GREEN << "OK" << RESET << " found: " << expected << std::endl;
		else
		{
			std::cout << RED << "KO" << RESET << std::endl;
			std::cout << "expected: " << expected << std::endl;
			std::cout << "recieved: " << readBuffer << std::endl;
		}
	}
	else
		std::cout << RED << "ERROR with curl" << RESET << std::endl;
	i++;
}

static void curl_post(const std::string& url, const std::string& expected)
{
	static int i = 1;
	long statuscode;
	CURL *curl;
	struct curl_slist *host = NULL;
	host = curl_slist_append(NULL, "webserv:80:127.0.0.1");
	curl_slist_append(host, "server1:6000:127.0.0.1");
	curl_slist_append(host, "server2:8080:127.0.0.1");
	CURLcode res;
	std::string readBuffer;
	curl = curl_easy_init();
	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_RESOLVE, host);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "This is the content of my new file.");
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		if(res == CURLE_OK)
		{
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statuscode);
		}
		curl_easy_cleanup(curl);
		std::cout << YELLOW << "Test " << i << ": " << url << RESET << std::endl;
		if (readBuffer.compare(expected) == 0)
			std::cout << GREEN << "OK" << RESET << std::endl;
		else if (nothrow_stol(expected) == statuscode)
			std::cout << GREEN << "OK" << RESET << " status code: " << statuscode << std::endl;
		else if (readBuffer.find(expected) != std::string::npos)
			std::cout << GREEN << "OK" << RESET << " found: " << expected << std::endl;
		else
		{
			std::cout << RED << "KO" << RESET << std::endl;
			std::cout << "expected: " << expected << std::endl;
			std::cout << "recieved: " << readBuffer << std::endl;
		}
	}
	else
		std::cout << RED << "ERROR with curl" << RESET << std::endl;
	i++;
}

static void curl_get(const std::string& url, const std::string& expected)
{
	static int i = 1;
	long statuscode;
	CURL *curl;
	struct curl_slist *host = NULL;
	host = curl_slist_append(NULL, "webserv:80:127.0.0.1");
	curl_slist_append(host, "server1:6000:127.0.0.1");
	curl_slist_append(host, "server2:8080:127.0.0.1");
	CURLcode res;
	std::string readBuffer;
	curl = curl_easy_init();
	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_RESOLVE, host);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		if(res == CURLE_OK)
		{
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statuscode);
		}
		curl_easy_cleanup(curl);
		std::cout << YELLOW << "Test " << i << ": " << url << RESET << std::endl;
		if (readBuffer.compare(expected) == 0)
			std::cout << GREEN << "OK" << RESET << std::endl;
		else if (nothrow_stol(expected) == statuscode)
			std::cout << GREEN << "OK" << RESET << " status code: " << statuscode << std::endl;
		else if (readBuffer.find(expected) != std::string::npos)
			std::cout << GREEN << "OK" << RESET << " found: " << expected << std::endl;
		else
		{
			std::cout << RED << "KO" << RESET << std::endl;
			std::cout << "expected: " << expected << std::endl;
			std::cout << "recieved: " << readBuffer << std::endl;
		}
	}
	else
		std::cout << RED << "ERROR with curl" << RESET << std::endl;
	i++;
}

int main(void)
{
	
	//////// GET
	std::cout << BLUE << "<<<<<<<<<<<<<<<<<<<<<<GET>>>>>>>>>>>>>>>>>>>>>>" << RESET << std::endl;
	curl_get("http://webserv", "content of index.html in root");
	curl_get("http://webserv:80", "content of index.html in root");
	curl_get("http://webserv/route/dir/file", "content of file in dir");
	curl_get("http://webserv/route/cgi/file", "content of file in cgi");
	// curl_get("http://server1:6000", "content of file in server1");
	// curl_get("http://server2/route/file", "xxx");
	// curl_get("http://server2:8080/route/file", "content of file in server2");
	// curl_get("http://server2:8081/route/file", "content of file in server2");
	// curl_get("http://server2/route/dir/file", "content of file in dir");
	curl_get("http://webserv/route/dir/file.cgi", "CONTENT OF FILE.CGI IN DIR");
	curl_get("http://webserv/route/dir/file.ext", "content of file.ext in extdir");
	curl_get("http://webserv/route/dir/norfile", "403");
	curl_get("http://webserv/route/nordir/file", "content of file in nordir");
	curl_get("http://webserv/route/nowdir/file", "content of file in nowdir");
	curl_get("http://webserv/route/noxdir/file", "404"); // kind of special
	curl_get("http://webserv/route/nordir/subdir/file", "content of file in nordirsubdir");
	curl_get("http://webserv/route/nowdir/subdir/file", "content of file in nowdirsubdir");
	curl_get("http://webserv/route/noxdir/subdir/file", "404"); // kind of special
	curl_get("http://webserv/route/dir/nowfile", "content of nowfile in dir");
	curl_get("http://webserv/route/dir/nonexistingfile", "404");
	curl_get("http://webserv/route/dir/nonexistingdir/", "404");
	curl_get("http://webserv/index", "404");
	curl_get("http://webserv/index/", "content of index.html in index");
	curl_get("http://webserv/index/custom/", "content of custom_index.html in custom");
	curl_get("http://webserv/index/no/autoindex/", "autoindex123");
	curl_get("http://webserv/index/no/autoindex/nopermission/", "404"); // kind of special
	curl_get("http://webserv/index/no/noautoindex/", "404");
	//////// POST
	std::cout << BLUE << "<<<<<<<<<<<<<<<<<<<<<<POST>>>>>>>>>>>>>>>>>>>>>>" << RESET << std::endl;
	curl_post("http://webserv/uploads/new.txt", "201");
	curl_post("http://webserv/uploads/new.txt", "201");
	curl_post("http://webserv/uploads/new.cgi", "THIS IS THE CONTENT OF MY NEW FILE.");
	//////// DELETE
	std::cout << BLUE << "<<<<<<<<<<<<<<<<<<<<<<DELETE>>>>>>>>>>>>>>>>>>>>>>" << RESET << std::endl;
	curl_delete("http://webserv/uploads/new.txt", "204");

}

//c++ curl_example.cpp -o curl_example -lcurl && ./curl_example