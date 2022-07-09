#include "Response.hpp"
#include "Request.hpp"

#include <iostream>

static void staticTestRequest (void)
{
	try
	{
		Request request("");
	}
	catch (std::exception & e)
	{
		std::cerr << e.what() << std::endl;
	}
	Request request("GET /index.html HTTP/1.1\nheaderfield1: value1\nheaderfield2: value2\n\nbody");
	std::cout << "HTTP request header:\n" << request << std::endl;
}

static void staticTestResponse (void)
{
	try
	{
		Response response("HTTP/1.1", -2);
	}
	catch (std::exception & e)
	{
		std::cerr << e.what() << std::endl;
	}
	Response response("HTTP/1.1", 200);
	std::cout << "HTTP response header:\n" << response.constructHeader() << std::endl;
}

int main (void)
{
	staticTestResponse();
	staticTestRequest();
}

//c++ -I ../inc/ test.cpp Message.cpp Response.cpp Request.cpp