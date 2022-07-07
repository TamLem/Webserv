#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#ifdef __APPLE__
	#define PLATFORM "macOS"
#else
	#define PLATFORM "Linux"
#endif

// this was the first try of a socket connection

int main() {

	int sockfd;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	const int port = 8080;

	std::cout << "This webserv is running on " << PLATFORM << std::endl;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cout << "Error creating socket" << std::endl;
		return 1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(serv_addr.sin_zero, '0', sizeof(serv_addr.sin_zero));

	if((bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
		std::cout << "Error binding socket" << std::endl;
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
		std::string header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
		std::string body = "<html><body><h1>Hello World</h1></body></html>";
		std::string response = header + body;
		if (write(new_sockfd, response.c_str(), response.size()) < 0) {
			std::cout << "Error writing to socket" << std::endl;
			return 1;
		}
		close(new_sockfd);
	}
	close(sockfd);

	return 0;
}