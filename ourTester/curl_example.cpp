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
	CURL *curl;
	CURLcode res;
	std::string readBuffer;
	curl = curl_easy_init();
	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if (readBuffer.compare(expected) == 0)
			std::cout << GREEN << "OK" << RESET << std::endl;
		else if (readBuffer.find(expected) != std::string::npos)
			std::cout << GREEN << "OK" << RESET << " found: " << expected << std::endl;
		else
			std::cout << RED << "KO" << RESET << std::endl;
	}
	else
		std::cout << RED << "ERROR with curl" << RESET << std::endl;
}

int main(void)
{
	myFunction("http://localhost:8080", "content of index.html in ourTesterRoot");
	myFunction("http://localhost:8080/nonexistingfile", "Error 404");
	myFunction("http://localhost:8080/nonexistingdir/", "Error 404");
}

//c++ curl_example.cpp -o curl_example -lcurl && ./curl_example