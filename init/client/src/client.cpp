#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>


int main()
{

	int sockfd;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cout << "Error creating socket" << std::endl;
		return 1;
	}

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8000);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(serv_addr.sin_zero, '0', sizeof(serv_addr.sin_zero));

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		std::cout << "Error connecting to socket" << std::endl;
		return 1;
	}
	else
		std::cout << "Connected to socket" << std::endl;
	char buffer[10000] = {0};
	char read_buffer[10000] = {0};
	int msg_size = 0;
	while ((msg_size = read(0, buffer, sizeof(buffer))) > 0)
	{
		if (write(sockfd, buffer, msg_size) < 0)
		{
			std::cout << "Error writing to socket" << std::endl;
			return 1;
		}
		if (read(sockfd, read_buffer, sizeof(read_buffer)) < 0)
		{
			std::cout << "Error reading from socket" << std::endl;
			return 1;
		}
		else
			std::cout << "Message received: " << read_buffer << std::endl;
		memset(buffer, 0, sizeof(buffer));
	}

	return 0;
}