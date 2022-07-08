#include "baseloop.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>

using namespace BaseLoop;
using namespace std;

struct TcpConn
{
	int fd;
	loop_event_data_t loop_data;

	TcpConn(int fd)
	{
		this->fd = fd;
		this->loop_data.data = this;
		this->loop_data.fd = fd;
	}
};

class TcpEcho: protected EventLoop
{
public:
	TcpEcho() {}

	void start(std::string &address)
	{
		this->init_loop();
		this->listen_tcp(address);
		this->run_loop();
	}

protected:
	/// callback for accepting connection here
	void acceptable(loop_event_data_t *data, int fd)
	{
		auto conn = new TcpConn(fd);
		this->register_handle(&conn->loop_data);
		this->make_readable(&conn->loop_data);
		cout << "Connection Accepted: " << fd << endl;
	};

	/// callback for reading data from socket here
	bool readable(loop_event_data_t *data, char *buffer, size_t read_len)
	{
		cout << YELLOW << "Read Data ->" << RESET << string(buffer, read_len) << YELLOW << "<- Read Data" << RESET << endl;
		// if (strnstr(buffer, "GET", 3) && strnstr(buffer, "favicon.ico", 20))
		if (strstr(buffer, "GET") && strstr(buffer, "favicon.ico")) //this is for linux if the above des not work
		{
			cout << GREEN << "@@@@@@ GET favicon.ico @@@@@" << RESET << endl << endl;
			std::ifstream input("images/favicon.ico", std::ios::binary);
			if (input.is_open())
			{
				filebuf *pbuf = input.rdbuf();
				std::size_t size = pbuf->pubseekoff(0,input.end,input.in);
				// cout << size << endl;
				pbuf->pubseekpos(0,input.in);
				char *out_buffer = new char[size];
				pbuf->sgetn (out_buffer,size);
				input.close();
				dprintf(data->fd, "HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: image/vdn.microsoft.icon\nContent-Length: %lu\n\n", size + 86);
				write(data->fd, out_buffer, size);
				delete[] out_buffer;
			}
			else
				std::cerr << RED << "image favicon.ico not found" << RESET << endl;
			close(data->fd);
		}
		// else if (strnstr(buffer, "GET", 3) && strnstr(buffer, "/images/large.jpg", 30))
		else if (strstr(buffer, "GET") && strstr(buffer, "/images/large.jpg")) //this is for linux if the above des not work
		{
			cout << GREEN << "@@@@@@ GET large.img @@@@@" << RESET << endl << endl;
			std::ifstream input("/Users/tblaase/Documents/webserv/init/server/images/large.jpg", std::ios::binary);
			filebuf *pbuf = input.rdbuf();
			std::size_t size = pbuf->pubseekoff(0,input.end,input.in);
			pbuf->pubseekpos(0,input.in);
			char *out_buffer = new char[size];
			pbuf->sgetn (out_buffer,size);
			input.close();
			dprintf(data->fd, "HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: image/jpg\nContent-Length: %lu\n\n", size + 86);
			write(data->fd, out_buffer, size);
			delete[] out_buffer;
			close(data->fd);
		}
		// else if (strnstr(buffer, "GET", 3))
		else if (strstr(buffer, "GET")) //this is for linux if the above des not work
		{
			cout << GREEN << "@@@@@@ GET index @@@@@" << RESET << endl << endl;
			std::ifstream myfile;
			myfile.open("/Users/tblaase/Documents/webserv/init/server/pages/index.html");
			std::string myline;
			std::string out_buffer;
			std::size_t size = 0;
			if (myfile.is_open())
			{
				while (myfile.good())
				{ // equivalent to myfile.good()
					std::getline (myfile, out_buffer);
					myline.append(out_buffer + "\n");
				}
				size = myline.length();
				dprintf(data->fd, "HTTP/1.1 200 OK\nServer: localhost:8080\nContent-Type: text/html\nContent-Length: %lu\n\n", size);
				write(data->fd, myline.c_str(), myline.length());
				myfile.close();
			}
			else
			{
				std::cerr << RED << "Couldn't open file" << endl;
				perror(NULL);
				std::cerr << RESET;
				dprintf(data->fd, "HTTP/1.1 404 \nServer: localhost:8080\nContent-Type: image/jpg\nContent-Length: %d\n\n", 86);
			}
			close(data->fd);
		}
		else
		{
			std::cout << "not the correct key" << std::endl;
			return false;
		}
		return true;
	};

	/// callback for writing data to socket
	void writable(loop_event_data_t *data)
	{
		this->make_readable(data);
	};

	/// callback function for getting connection close event
	void closed(loop_event_data_t *data)
	{
		auto conn = (TcpConn *) data->data;
		delete conn;

		cout << "Connection closed: " << data->fd << endl;
	};
};

// here we need to accept the config file
int main()
{
	// parse_config(argv[1]);
	string address("127.0.0.1:8080");
	TcpEcho echo;
	echo.start(address);
}
