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

static void myFunction(const std::string& url, const std::string& expected)
{
	static int i = 1;
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
		curl_easy_cleanup(curl);
		std::cout << YELLOW << "Test " << i << ": " << url << RESET << std::endl;
		if (readBuffer.compare(expected) == 0)
			std::cout << GREEN << "OK" << RESET << std::endl;
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
	// myFunction("http://webserv", "content of file in dir");
	// myFunction("http://webserv:80", "content of file in dir");
	// myFunction("http://webserv/route/dir/file", "content of file in dir");
	// myFunction("http://webserv/route/cgi/file", "content of file in cgi");
	// myFunction("http://server1:6000", "content of file in server1");
	// myFunction("http://server2/route/file", "xxx");
	// myFunction("http://server2:8080/route/file", "content of file in server2");
	// myFunction("http://server2:8081/route/file", "content of file in server2");
	// myFunction("http://server2/route/dir/file", "content of file in dir");
	// myFunction("http://webserv/route/dir/file.cgi", "CONTENT OF FILE.CGI IN DIR");
	myFunction("http://webserv/route/dir/file.ext", "content of file.ext in extdir");
	myFunction("http://webserv/route/dir/noxfile", "content of noxfile in dir");
	myFunction("http://webserv/route/dir/norfile", "Error 403");
	myFunction("http://webserv/route/dir/nowfile", "content of nowfile in dir");
	myFunction("http://webserv/route/dir/nonexistingfile", "Error 404");
	myFunction("http://webserv/route/dir/nonexistingdir/", "Error 404");
}

//c++ curl_example.cpp -o curl_example -lcurl && ./curl_example