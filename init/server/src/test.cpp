#include "Response.hpp"
#include "Request.hpp"

#include <iostream>

void test_request (void)
{
	Request request;
	request.setMethod("GET");
	request.setUrl("/index.html");
	request.setProtocol("HTTP/1.1");
	std::cout << "HTTP request header:\n" << request << std::endl;
}

void test_response (void)
{
	Response response;
	// std::cout << response.messages[100] << std::endl;
	response.setProtocol("HTTP/1.1");
	response.setStatus(200);
	std::cout << "HTTP response header:\n" << response.construct_header() << std::endl;
}

int main (void)
{
	test_response();
	test_request();
}

//c++ -I ../inc/ test.cpp Response.cpp Request.cpp