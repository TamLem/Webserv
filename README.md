# webserv - An HTTP Web Server

This is an implementation of an HTTP Web Server, in **C++98** using **C** socket libraries<br>
and our solution for the webserv project of the 42 Core Curriculum.


# Contents
- [Features](https://github.com/TamLem/Webserv#features)
- [Basics](https://github.com/TamLem/Webserv#Basics)
- [Config File](https://github.com/TamLem/Webserv#config-file)
- [Tester](https://github.com/TamLem/Webserv#tester)


## Features
- user provided config file
- IO multiplexing: multiplexing of multiple sockets, using kqueue
- CGI/1.1 (dynamic content)
- HTTP/1.1 Protocol
  - downloading a file, i.e. showing a static website (GET)
  - uploading a file (POST)
  - deleting a file (DELETE)


## Basics
In this project we were challanged to create a webserver with quite a few functionalities that complies to the HTTP/1.1 standard.<br>
This, on its  own is already a challenge when you started coding only 1.5 years ago.<br>
But to challange us even more we had to mainly use c++98 + and some c functions for alll the cases c++98 has no alternative like sockets and directory handling.<br>
First challenge was to have a config file that has a lot of functionalities and works kinda like a config file for [nginx](https://www.nginx.com/resources/wiki/start/topics/examples/full/).<br>
The methods to implement where `GET` `POST` and `DELETE`.<br>
There had to be autoindexing as well as directory/file routing.
Especially the directory/file routing is important to hide the actual directory structure of our server to the user that visites a website that is hosted on our server.<br>
One big and also complicated part was the implementation of chunked requests, because of the multiplexing.<br>
For our multiplexing we used kqueue, this enabled us to get the events (i.e. client connect, client send a request) directly from the kernel.<br>
For this reason our webserv is only running on macOS, since kqueue is not implemented in linux.<br>
The implementation of CGI's was a big part of the work too. On our webserv you can have as many cgi-extensions working as you like, as long as the cgi-executable is supplied.<br>
And last but not least we had to make sure our webserv never crashes and gives back appropriate error codes if something gous wrong.<br>


## Config File
Theses following variables can be set inside the config file:<br>
- hostname
- mutliple ports
- turn on/off automatic directory listing
- setup of an index file (server-wide and directory specific)
- redirections for
  - certain directories
  - certain file-extensions
- allowing methods for certain directories/file-extensions
- setup for the cgi
- setup of max_client_body_size (max number of bytes allowed in a single file/chunk)


Also you are able to setup multiple servers in the same config file.<br>
This will lead the webserver to be able to run multiple configs at the same time and applying them depending on which host was called in the request.<br>


[here](https://github.com/TamLem/Webserv/server/config/www.conf) you can find an example config with explanations, or check [this README](https://github.com/TamLem/Webserv/server/config/README.md), which explains our config-file in-depth.<br>

## Tester
Inside the directory [ourTester]() you are able to find our own tester that we created to test our project with.<br>
For some tests it uses a custom client (also written in c++ and c) and for some tests it uses curl.<br>
For more information on how to use it check [this README](https://github.com/TamLem/Webserv/ourTester/README.md) of the tester.<br>


