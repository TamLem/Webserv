# webserv - An HTTP Web Server 

This is an implementation of an HTTP Web Server, in **C++** using **C** socket libraries. 

Features: 
- IO multiplexing: multiplexing of multiple sockets, using kqueue 
- Multiple servers
- mutliple ports
- CGI/1.1
- HTTP Protocol
  - GET, POST, and DELETE methods
- user provided config file
- file upload and download
- automatic directory listing