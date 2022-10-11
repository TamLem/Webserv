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
#define FILE_LINE (std::string(__FILE__) + std::string(":") + std::to_string(__LINE__))


#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
// #include <string.h>
#include <arpa/inet.h>

#define PORT 80
#define NONE -1
#define EMPTY -2

static long nothrow_stol(const std::string& string)
{
	long ret;
	try {
		ret = stol(string);
	}
	catch (std::exception &e)
	{
		return (NONE);
	}
	return (ret);
}

static void evaluation(const std::string& testcase, const std::string& readBuffer, const std::string& expected, long statuscode, const std::string &testLocation)
{
	static int i = 1;

	std::cout << YELLOW << "Test " << i << ": " << testcase << BLUE << "\tfrom: " << testLocation << RESET << std::endl;
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
	i++;
}

static void my_request(const std::string& request, const std::string& expected, const std::string &testLocation)
{
	int sock = 0;
	long valread;
	struct sockaddr_in serv_addr;
	char readBuffer[1024] = {0};
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cout << "\n Socket creation error" << std::endl;
		return ;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		std::cout << "\nInvalid address/ Address not supported" << std::endl;
		return ;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		std::cout << "\nConnection Failed" << std::endl;
		return ;
	}
	send(sock, request.c_str(), request.length(), 0);
	valread = read( sock , readBuffer, 1024);
	evaluation(request.substr(0, request.find_first_of('\n') - 1), readBuffer, expected, EMPTY, testLocation);
}


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

static void curl_delete(const std::string& url, const std::string& expected, const std::string &testLocation)
{
	long statuscode;
	CURL *curl;
	struct curl_slist *host = NULL;
	host = curl_slist_append(NULL, "webserv:80:127.0.0.1");
	curl_slist_append(host, "webserv:5500:127.0.0.1");
	curl_slist_append(host, "server1:6000:127.0.0.1");
	curl_slist_append(host, "server2:80:127.0.0.1");
	curl_slist_append(host, "server2:8080:127.0.0.1");
	curl_slist_append(host, "server2:8081:127.0.0.1");
	curl_slist_append(host, "server2:127.0.0.1");
	curl_slist_append(host, "nonexistingserver:8080:127.0.0.1");
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
		evaluation(url, readBuffer, expected, statuscode, testLocation);
	}
	else
		std::cout << RED << "ERROR with curl" << RESET << std::endl;
}

static void curl_post(const std::string& url, const std::string& expected, const std::string &testLocation)
{
	long statuscode;
	CURL *curl;
	struct curl_slist *host = NULL;
	host = curl_slist_append(NULL, "webserv:80:127.0.0.1");
	curl_slist_append(host, "webserv:5500:127.0.0.1");
	curl_slist_append(host, "server1:6000:127.0.0.1");
	curl_slist_append(host, "server2:80:127.0.0.1");
	curl_slist_append(host, "server2:8080:127.0.0.1");
	curl_slist_append(host, "server2:8081:127.0.0.1");
	curl_slist_append(host, "server2:127.0.0.1");
	curl_slist_append(host, "nonexistingserver:8080:127.0.0.1");
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
		evaluation(url, readBuffer, expected, statuscode, testLocation);
	}
	else
		std::cout << RED << "ERROR with curl" << RESET << std::endl;
}

static void curl_get(const std::string& url, const std::string& expected, const std::string &testLocation)
{
	long statuscode;
	CURL *curl;
	struct curl_slist *host = NULL;
	host = curl_slist_append(NULL, "webserv:80:127.0.0.1");
	curl_slist_append(host, "webserv:5500:127.0.0.1");
	curl_slist_append(host, "server1:6000:127.0.0.1");
	curl_slist_append(host, "server2:80:127.0.0.1");
	curl_slist_append(host, "server2:8080:127.0.0.1");
	curl_slist_append(host, "server2:8081:127.0.0.1");
	curl_slist_append(host, "server2:127.0.0.1");
	curl_slist_append(host, "nonexistingserver:8080:127.0.0.1");
	CURLcode res;
	std::string readBuffer;
	curl = curl_easy_init();
	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_RESOLVE, host);
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		if(res == CURLE_OK)
		{
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statuscode);
		}
		curl_easy_cleanup(curl);
		evaluation(url, readBuffer, expected, statuscode, testLocation);
	}
	else
		std::cout << RED << "ERROR with curl" << RESET << std::endl;
}

int main(void)
{
	// BAD REQUEST TESTS
	std::cout << BLUE << "<<<<<<<<<<<<<<<<<<<<<<INPUT>>>>>>>>>>>>>>>>>>>>>>" << RESET << std::endl;
	my_request("GET / HTTP/1.1\r\nHost: webserv\r\n\r\n", "content of index.html in root", FILE_LINE);
	my_request("GET /  HTTP/1.1\r\nHost: webserv\r\n\r\n", "400", FILE_LINE);
	my_request("GET  / HTTP/1.1\r\nHost: webserv\r\n\r\n", "400", FILE_LINE);
	my_request(" GET / HTTP/1.1\r\nHost: webserv\r\n\r\n", "400", FILE_LINE);
	my_request("GET / HTTP/1.1\r\nHost:webserv\r\n\r\n", "content of index.html in root", FILE_LINE);
	my_request("GET / HTTP/1.1\r\n Host: webserv\r\n\r\n", "400", FILE_LINE);
	my_request("GET / HTTP/1.1\r\nHost: webserv \r\n\r\n", "content of index.html in root", FILE_LINE);
	my_request("GET / HTTP/1.1\r\nHost:  webserv  \r\n\r\n", "400", FILE_LINE); // AE only one whitespace
	my_request("GET / HTTP/1.1\r\nHost:      webserv      \r\n\r\n", "400", FILE_LINE); // AE only one whitespace
	my_request("GET / HTTP/1.1\r\nHost:webserv      \r\n\r\n", "400", FILE_LINE); // AE only one whitespace
	my_request("GET / HTTP/1.1\r\nHost:    webserv\r\n\r\n", "400", FILE_LINE); // AE only one whitespace
	my_request("GET / HTTP/1.1\r\nHost : webserv\r\n\r\n", "400", FILE_LINE);
	my_request("GET /route/dir/%25file HTTP/1.1\r\nHost: webserv\r\n\r\n", "content of %file in dir", FILE_LINE);
	my_request("GET /route/dir/%2myfile HTTP/1.1\r\nHost: webserv\r\n\r\n", "400", FILE_LINE);
	my_request("GET /ü-ei HTTP/1.1\r\nHost: webserv\r\n\r\n", "400", FILE_LINE); //not encoded
	// my_request("", "408", FILE_LINE); // timeout
	my_request("\n", "400", FILE_LINE);
	my_request(" ", "400", FILE_LINE); // AE problem
	my_request("GET . HTTP/1.1\r\nHost: webserv\r\n\r\n", "content of index.html in root", FILE_LINE); // AE garbage
	my_request("GET .. HTTP/1.1\r\nHost: webserv\r\n\r\n", "content of index.html in root", FILE_LINE); // AE garbage
	my_request("GET ... HTTP/1.1\r\nHost: webserv\r\n\r\n", "404", FILE_LINE); // AE garbage
	my_request("GET .../README.md HTTP/1.1\r\nHost: webserv\r\n\r\n", "404", FILE_LINE); // AE ultra dangerous!!!
	my_request("GET HTTP/1.1\r\nHost: webserv\r\n\r\n", "400", FILE_LINE);
	my_request("GET / HTTP/1.0\r\nHost: webserv\r\n\r\n", "505", FILE_LINE);
	my_request("GET / HTTP/1.1\r\nHost: webserv\r\nHost: webserv\r\n\r\n", "400", FILE_LINE);
	my_request("GET / HTTP/1.1\r\nHost: webserv\r\ncontent-length: 0\r\ntransfer-encoding: chunked\r\n\r\n", "400", FILE_LINE); // AE is this even imlemented?
	my_request("GET / HTTP/1.1\r\nUser-Agent: Go-http-client/1.1\r\n\r\n", "400", FILE_LINE);
	// my_request("GET / HTTP/1.1\r\nHost: webserv\r\n\r", "408", FILE_LINE); // timeout
	my_request("GET / HTTP/1.1\nHost: webserv\r\n\r\n", "400", FILE_LINE);
	my_request("PST / HTTP/1.1\r\nHost: webserv\r\n\r\n", "501", FILE_LINE);
	my_request("POST /new.txt HTTP/1.1\r\nHost: webserv\r\n\r\n", "411", FILE_LINE);
	my_request("GET 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 HTTP/1.1\r\nHost: webserv\r\n\r\n", "414", FILE_LINE);
	my_request("POST /uploads/big.txt HTTP/1.1\r\nHost: webserv\r\ncontent-length: 200\r\n\r\n01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", "413", FILE_LINE);
	//////// GET
	std::cout << BLUE << "<<<<<<<<<<<<<<<<<<<<<<GET>>>>>>>>>>>>>>>>>>>>>>" << RESET << std::endl;
	curl_get("http://webserv", "content of index.html in root", FILE_LINE);
	curl_get("http://webserv:80", "content of index.html in root", FILE_LINE);
	curl_get("http://webserv/route/lol/", "content of different.html in lol", FILE_LINE);
	curl_get("http://webserv/route/lol/troll/", "content of index.html in troll", FILE_LINE);
	curl_get("http://webserv:5500", "", FILE_LINE); //  can't connect
	curl_get("http://webserv/route/dir/file", "content of file in dir", FILE_LINE);
	curl_get("http://webserv/route/cgi/file", "content of file in cgi", FILE_LINE);
	curl_get("http://server1:6000/route/file", "content of file in server1", FILE_LINE);
	curl_get("http://server1:6000/route/doesnotexist", "MY_CUSTOM_PAGE", FILE_LINE);
	curl_get("http://server1:6000/", "content of default.html in root", FILE_LINE);
	curl_get("http://server1:6000/index1/", "content of different.html in root", FILE_LINE);
	curl_get("http://server1:6000/index2/", "content of default.html in root", FILE_LINE);
	curl_get("http://server2/route/file", "405", FILE_LINE); // going for default server
	curl_get("http://nonexistingserver:8080/route/file", "405", FILE_LINE); // going for default server
	curl_get("http://server2:8080/route/file", "content of file in server2", FILE_LINE);
	curl_get("http://server2:8081/route/file", "content of file in server2", FILE_LINE);
	curl_get("http://webserv/route/dir/file.cgi", "CONTENT OF FILE.CGI IN DIR", FILE_LINE); // only POST triggers cgi, GET only returns file WRONG!
	curl_get("http://webserv/route/dir/file.ext", "content of file.ext in extdir", FILE_LINE);
	curl_get("http://webserv/route/dir/norfile", "403", FILE_LINE);
	curl_get("http://webserv/route/nordir/file", "content of file in nordir", FILE_LINE);
	curl_get("http://webserv/route/nowdir/file", "content of file in nowdir", FILE_LINE);
	curl_get("http://webserv/route/noxdir/file", "403", FILE_LINE);
	curl_get("http://webserv/route/nordir/subdir/file", "content of file in nordirsubdir", FILE_LINE);
	curl_get("http://webserv/route/nowdir/subdir/file", "content of file in nowdirsubdir", FILE_LINE);
	curl_get("http://webserv/route/noxdir/subdir/file", "403", FILE_LINE);
	curl_get("http://webserv/route/dir/nowfile", "content of nowfile in dir", FILE_LINE);
	curl_get("http://webserv/route/dir/nonexistingfile", "404", FILE_LINE);
	curl_get("http://webserv/route/dir/nonexistingdir/", "404", FILE_LINE);
	curl_get("http://webserv/index", "404", FILE_LINE);
	curl_get("http://webserv/index/", "content of index.html in index", FILE_LINE);
	curl_get("http://webserv/index/custom/", "content of custom_index.html in custom", FILE_LINE);
	curl_get("http://webserv/index/no/autoindex/", "autoindex123", FILE_LINE);
	curl_get("http://webserv/index/no/autoindex/nopermission/", "403", FILE_LINE);
	curl_get("http://webserv/index/no/autoindex/nonexisting/", "404", FILE_LINE);
	curl_get("http://webserv/index/no/noautoindex/", "404", FILE_LINE);
	//////// POST
	std::cout << BLUE << "<<<<<<<<<<<<<<<<<<<<<<POST>>>>>>>>>>>>>>>>>>>>>>" << RESET << std::endl;
	curl_post("http://server2:8080/new.txt", "405", FILE_LINE);
	curl_post("http://webserv/uploads/new.txt", "201", FILE_LINE);
	curl_post("http://webserv/uploads/.ext", "201", FILE_LINE); // not an extension
	curl_post("http://webserv/uploads/new.txt", "201", FILE_LINE);
	curl_post("http://webserv/uploads/newdir/", "400", FILE_LINE); // create directory is not possible
	curl_post("http://webserv/uploads/doesnotexist/new.txt", "404", FILE_LINE); // changed to be 404 instead of 400
	curl_post("http://webserv/uploads/cgi/new.txt", "403", FILE_LINE); // 403 because cgi directory has no permissions
	curl_post("http://webserv/uploads/new.cgi", "THIS IS THE CONTENT OF MY NEW FILE.", FILE_LINE);
	curl_post("http://webserv/uploads/new.cgi", "200", FILE_LINE); // RFC says 201 should only be returned on cration of a ressource, so no reason to return 201 on a cgi POST
	curl_post("http://webserv/uploads/file.cgi.not", "201", FILE_LINE);
	curl_post("http://webserv/uploads/.cgi", "201", FILE_LINE);
	curl_post("http://server1:6000/route/file.cgi", "500", FILE_LINE); //no executepermission for cgi executable
	curl_post("http://server2:8080/route/file.cgi", "500", FILE_LINE); //cgi executable doesn't exist
	//no cgi executable
	// no cgi permission
	//////// DELETE
	std::cout << BLUE << "<<<<<<<<<<<<<<<<<<<<<<DELETE>>>>>>>>>>>>>>>>>>>>>>" << RESET << std::endl;
	curl_delete("http://webserv/uploads/new.txt", "204", FILE_LINE);
	curl_delete("http://webserv/uploads/.ext", "204", FILE_LINE);
	curl_delete("http://webserv/uploads/newdir/", "204", FILE_LINE); // would have to be created first
	curl_delete("http://webserv/uploads/cgi/new.txt", "404", FILE_LINE); // cgi directory has no permissions
	curl_delete("http://webserv/uploads/file.cgi.not", "204", FILE_LINE);
	curl_delete("http://webserv/uploads/.cgi", "204", FILE_LINE);
	curl_delete("http://webserv/uploads/doesnotexist", "404", FILE_LINE);
	curl_delete("http://webserv/uploads/nonexisting.cgi", "404", FILE_LINE); // file to call the cgi on is not existing
	curl_get("http://webserv/uploads/nopermissionfile.cgi", "403", FILE_LINE); // file to call the cgi on has no permissions
	curl_post("http://webserv/uploads/nopermissionfile.cgi", "THIS IS THE CONTENT OF MY NEW FILE.", FILE_LINE); // file to call the cgi on has no permissions
	curl_delete("http://webserv/uploads/cgi/nopermissionfile.cgi", "404", FILE_LINE); // has no permissions inside the cgi directory
	curl_delete("http://webserv/uploads/todelete.cgi", "204", FILE_LINE);
}

//c++ curl_example.cpp -o curl_example -lcurl && ./curl_example

/*
42teser testcases
_________________________________________________
Test 1 GET http://localhost:8080/
GET / HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Accept-Encoding: gzip

Response:
HTTP/1.1 200 OK

_________________________________________________
Test 2 POST http://localhost:8080/ with a size of 0
POST / HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Transfer-Encoding: chunked
Content-Type: test/file
Accept-Encoding: gzip

Response:
HTTP/1.1 405 Method Not Allowed

_________________________________________________
Test 3 HEAD http://localhost:8080/
HEAD / HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1

Response:
HTTP/1.1 405 Method Not Allowed

_________________________________________________
Test 4 GET http://localhost:8080/directory
GET /directory HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Accept-Encoding: gzip

Response:
HTTP/1.1 200 OK

_________________________________________________
Test 5 GET http://localhost:8080/directory/youpi.bad_extension
GET /directory/youpi.bad_extension HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Accept-Encoding: gzip

Response:
HTTP/1.1 200 OK


_________________________________________________
Test 6 GET http://localhost:8080/directory/youpi.bla
GET /directory/youpi.bla HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Accept-Encoding: gzip

Response:
HTTP/1.1 201 Created

_________________________________________________
Test 7 GET Expected 404 on http://localhost:8080/directory/oulalala
GET /directory/oulalala HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Accept-Encoding: gzip

Response:
HTTP/1.1 404 Not Found

_________________________________________________
Test 8 GET http://localhost:8080/directory/nop
GET /directory/nop HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Accept-Encoding: gzip

Response:
HTTP/1.1 200 OK

_________________________________________________
Test 9 GET http://localhost:8080/directory/nop/
GET /directory/nop/ HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Accept-Encoding: gzip

Response:
HTTP/1.1 200 OK


_________________________________________________
Test 10 GET http://localhost:8080/directory/nop/other.pouic
GET /directory/nop/other.pouic HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Accept-Encoding: gzip

Response:
HTTP/1.1 200 OK

_________________________________________________
Test 11 GET Expected 404 on http://localhost:8080/directory/nop/other.pouac
GET /directory/nop/other.pouac HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Accept-Encoding: gzip

Response:
HTTP/1.1 404 Not Found

_________________________________________________
Test 12 GET Expected 404 on http://localhost:8080/directory/Yeah
GET /directory/Yeah HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Accept-Encoding: gzip

Response:
HTTP/1.1 404 Not Found

_________________________________________________
Test 13 GET http://localhost:8080/directory/Yeah/not_happy.bad_extension
GET /directory/Yeah/not_happy.bad_extension HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Accept-Encoding: gzip

Response:
HTTP/1.1 200 OK

_________________________________________________
Test 14 Put http://localhost:8080/put_test/file_should_exist_after with a size of 1000
PUT /put_test/file_should_exist_after HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Transfer-Encoding: chunked
Accept-Encoding: gzip

Response:
HTTP/1.1 201 Created

_________________________________________________
Test 15 Put http://localhost:8080/put_test/file_should_exist_after with a size of 10000000
PUT /put_test/file_should_exist_after HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Transfer-Encoding: chunked
Accept-Encoding: gzip

Response:
HTTP/1.1 201 Created

_________________________________________________
Test 16 POST http://localhost:8080/directory/youpi.bla with a size of 100000000
POST /directory/youpi.bla HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Transfer-Encoding: chunked
Content-Type: test/file
Accept-Encoding: gzip

_________________________________________________
Test 17 POST http://localhost:8080/directory/youpla.bla with a size of 100000000
POST /directory/youpla.bla HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Transfer-Encoding: chunked
Content-Type: test/file
Accept-Encoding: gzip

_________________________________________________
Test 18 POST http://localhost:8080/directory/youpi.bla with a size of 100000 with special headers
POST /directory/youpi.bla HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Transfer-Encoding: chunked
Content-Type: test/file
X-Secret-Header-For-Test: 1
Accept-Encoding: gzip

_________________________________________________
Test 19 POST http://localhost:8080/post_body with a size of 0
POST /post_body HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Transfer-Encoding: chunked
Content-Type: test/file
Accept-Encoding: gzip

_________________________________________________
Test 20 POST http://localhost:8080/post_body with a size of 100
POST /post_body HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Transfer-Encoding: chunked
Content-Type: test/file
Accept-Encoding: gzip

_________________________________________________
Test 21 POST http://localhost:8080/post_body with a size of 200
POST /post_body HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Transfer-Encoding: chunked
Content-Type: test/file
Accept-Encoding: gzip

_________________________________________________
Test 22 POST http://localhost:8080/post_body with a size of 101
POST /post_body HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Transfer-Encoding: chunked
Content-Type: test/file
Accept-Encoding: gzip

_________________________________________________
Test 23 multiple workers(5) doing multiple times(15): GET on /
GET / HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Accept-Encoding: gzip

_________________________________________________
Test 24 multiple workers(20) doing multiple times(5000): GET on /

_________________________________________________
Test 25 multiple workers(128) doing multiple times(50): GET on /directory/nop

_________________________________________________
Test 26 multiple workers(20) doing multiple times(5): Put on /put_test/multiple_same with size 1000000
PUT /put_test/multiple_same HTTP/1.1
Host: localhost:8080
User-Agent: Go-http-client/1.1
Transfer-Encoding: chunked
Accept-Encoding: gzip

_________________________________________________
Test 27 multiple workers(20) doing multiple times(5): Post on /directory/youpi.bla with size 100000000
*/