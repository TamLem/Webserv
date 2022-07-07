#include "Response.hpp"

#include <iostream>

int main (void)
{
	Response response;
	// std::cout << response.messages[100] << std::endl;
	response.setProtocol("HTTP/1.1");
	response.setStatus(200);
	std::cout << response.construct_header() << std::endl;
}

//c++ -I ../inc/ test.cpp Response.cpp