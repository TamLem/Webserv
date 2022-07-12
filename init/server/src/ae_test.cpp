#include "Response.hpp"
#include "Request.hpp"

#include <iostream>
#include <sstream> //ostringstream

// #define RED "\033[0;31m"
// #define GREEN "\033[0;32m"
// #define YELLOW "\033[0;33m"
// #define BLUE "\033[0;34m"
// #define BOLD "\033[1m"
// #define UNDERLINED "\033[4m"
// #define RESET "\033[0m"

static void staticTestRequest (void)
{
	std::cout << BOLD << YELLOW << __func__ << " in line " << __LINE__ << RESET << std::endl;
	std::string startLine = "GET /index.html HTTP/1.1\r\n";
	std::string body = "My body, my choice!";
	std::string header = "Content-Length: ";
	//https://m.cplusplus.com/articles/D9j2Nwbp/
	header += static_cast<std::ostringstream*>( &(std::ostringstream() << body.length()) )->str();
	header += "\r\n";
	header += "headerfield1: value1\nheaderfield2: value2\r\n\r\n";
	std::cout << BOLD << "Exception Test:" << RESET << std::endl;
	try
	{
		Request request("");
	}
	catch (std::exception & e)
	{
		std::cerr << RED << e.what() << RESET << std::endl;
	}
	std::cout << BOLD << "Request Header Test:" << RESET << std::endl;
	Request request(startLine + header + body);
	std::cout << request;// << std::endl;
}

static void staticTestResponse (void)
{
	std::cout << BOLD << YELLOW << __func__ << " in line " << __LINE__ << RESET << std::endl;
	std::cout << BOLD << "Exception Test:" << RESET << std::endl;
	try
	{
		Response response("HTTP/1.1", -2, 40, "url");
	}
	catch (std::exception & e)
	{
		std::cerr << RED << e.what() << RESET << std::endl;
	}
	std::cout << BOLD << "Response Header Test:" << RESET << std::endl;
	Response response("HTTP/1.1", 200, 40, "/url.html");
	std::cout << response.constructHeader();// << std::endl;
}

int main (void)
{
	staticTestResponse();
	staticTestRequest();
}

//c++ -I ../inc/ ae_test.cpp Message.cpp Response.cpp Request.cpp -std=c++98 -Wall -Wextra -Werror