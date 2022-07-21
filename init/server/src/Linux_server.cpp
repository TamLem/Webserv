#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <stdio.h>

#include "Request.hpp"
#include "Response.hpp"
#include "Base.hpp"


// Forbidden includes
#include <errno.h>

// this was the first try of a socket connection, now is only used to have a working linux compatible simple server

int main() {

	int sockfd;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	const int port = 8080;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cout << "Error creating socket" << std::endl;
		return 1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(serv_addr.sin_zero, '0', sizeof(serv_addr.sin_zero));

	if((bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
		std::cout << RED << "Error binding socket" << std::endl;
		perror("bind");
		std::cerr << RESET;
		return 1;
	}

	if (listen(sockfd, 5) < 0) {
		std::cout << "Error listening on socket" << std::endl;
		return 1;
	}

	int new_sockfd;
	while(true) {
		socklen_t clilen = sizeof(cli_addr);
		if ((new_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) < 0) {
			std::cout << "Error accepting connection" << std::endl;
			return 1;
		}
		std::cout << "Connection accepted" << std::endl;
		char buffer[10000] = {0};
		if (read(new_sockfd, buffer, sizeof(buffer)) < 0) {
			std::cout << "Error reading from socket" << std::endl;
			return 1;
		}
		std::cout << "Message received: " << buffer << std::endl;
		Request newRequest(buffer);
		Response newResponse("HTTP/1.1", 200, new_sockfd, newRequest.getUri());
		// std::cout << newResponse.constructHeader();
		newResponse.sendResponse();
		close(new_sockfd);
	}
	close(sockfd);

	return 0;
}