# webserv - An HTTP Web Server

This is an implementation of an HTTP Web Server, in **C++98** using **C** socket libraries.

Features:
- user provided config file (tried to stay close to nginx's config file)
  - Multiple servers
  - mutliple ports
  - automatic directory listing
  - redirections for
    - certain directories
    - certain file-extensions
  - setup for the cgi
  - allowing methods for certain directories/file-extensions
- IO multiplexing: multiplexing of multiple sockets, using kqueue
- CGI/1.1
- HTTP/1.1 Protocol
  - uploading a file (POST)
  - downloading a file, i.e. showing a static website (GET)
  - deleting a file (DELETE)