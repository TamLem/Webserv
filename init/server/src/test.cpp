#include "Response.hpp"
#include "Request.hpp"

#include <iostream>

static void static_test_request (void)
{
	try
	{
		Request request("");
	}
	catch (std::exception & e)
	{
		std::cerr << e.what() << std::endl;
	}
	Request request("abc");
	// request.setMethod("GET");
	// request.setUrl("/index.html");
	// request.setProtocol("HTTP/1.1");
	std::cout << "HTTP request header:\n" << request << std::endl;
}

static void static_test_response (void)
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
	// std::cout << response.messages[100] << std::endl;
	// response.setProtocol("HTTP/1.1");
	// response.setStatus(200);
	std::cout << "HTTP response header:\n" << response.construct_header() << std::endl;
}

int main (void)
{
	static_test_response();
	static_test_request();
}

//c++ -I ../inc/ test.cpp Response.cpp Request.cpp