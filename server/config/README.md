# Explanations on how to use our config-file
All key value pairs/triples have to be contained within the same line and have to be seperated by nothing else than one whitespace character, either ` ` or `\t`<br>

Those are all the options:
- [server](https://github.com/TamLem/Webserv/blob/master/server/config/README.md#server)
- [listen](https://github.com/TamLem/Webserv/blob/master/server/config/README.md#listen)
- [root](https://github.com/TamLem/Webserv/blob/master/server/config/README.md#root)
- [server_name](https://github.com/TamLem/Webserv/blob/master/server/config/README.md#server_name)
- [autoindex](https://github.com/TamLem/Webserv/blob/master/server/config/README.md#autoindex)
- [index_page](https://github.com/TamLem/Webserv/blob/master/server/config/README.md#index_page)
- [client_max_body_buffer_size](https://github.com/TamLem/Webserv/blob/master/server/config/README.md#client_max_body_buffer_size)
- [client_max_body_size](https://github.com/TamLem/Webserv/blob/master/server/config/README.md#client_max_body_size)
- [cgi](https://github.com/TamLem/Webserv/blob/master/server/config/README.md#cgi)
- [cgi_bin](https://github.com/TamLem/Webserv/blob/master/server/config/README.md#cgi_bin)
- [location](https://github.com/TamLem/Webserv/blob/master/server/config/README.md#location)
- [error_page](https://github.com/TamLem/Webserv/blob/master/server/config/README.md#error_page)


## server
`server {` this is the only way to start a server-block<br>
`}` a server-block needs to be closed before the end of the config-file or and before a new server-block starts.<br>
Those two lines can not contain any additional content other than comments.<br>
Comments can be initialised by either a `#`  or a `;` and will set everything behind it on the same line as comment.<br>


## listen
This is how you can set up the ports inside a server-block.<br>
There can be multiple ports in one server-block and the same ports can be used in different servers-blocks.<br>

```
listen 8080
listen 8081;
listen 8082;
```

## root
Here you can setup the root location of your server.<br>
```
root /server/data/
```

## server_name
This will be the hostname of your server.<br>
It can not be `default` and it has to be unique for that config-file.<br>
Special to this hostname will be that you are not really able to set it to anything you like as long as you are running it in 42 school and want to access the server via your browser, because for that to work you would need write access to the /etc/hosts file.<br>
But for running tests with `curl` you will be able to set any hostname you like, as long as you set the appropriate flags in curl.<br>
```
server_name weebserv
```

## autoindex
Here is the global option for autoindex, it can either be true or false, with false beeing the default value.<br>
If true this will enable directory-listing if a directory was requested and no index-file is present.<br>
```
autoindex true
```

## index_page
The index_page will decide what index file to show if root was requested.<br>
This can be any file you wish, it just has to be accessible.<br>
```
index_page index.html
```

## client_body_buffer_size
The client_body_buffer_size controlls the max number of byte to read from a body of a request at once.<br>
The default of this value will be 64kB<br>
```
client_body_buffer_size 104655454
```

## client_max_body_size
This will limit the maximum accepted size of either a full body if it is not a chunked request or the size of one chunk if it is a chunked request.<br>
Default will be set to 1GB<br>
```
client_max_body_size 1000000000
```

## cgi
Here you can set the file-extension(s) that should be treated as cgi.<br>
With that you also need to supply the appropriate path to the executable.<br>
```
cgi .file_extension handler_exe
cgi .php ./server/data/cgi-bin/php-cgi
```

## cgi_bin
This will define the path where the cgi binaries will be stored.<br>
```
cgi_bin	/server/data/cgi-bin/
```

## location
The location-blocks are a powerfull tool to change file- and directory-routing to hide the backend directory structure to the user of a website.<br>
It also controlls allowed methods, autoindexing and the index_page of a certain directory.<br>
The following location-block will change `localhost:8080/cgi/` and change it in the backend to `localhost:8080/server/data/cgi-bin/`.<br>
And for this request it will show the `index.cgi`.<br>
By default no methods will be allowed for a location when defined, autindex will be false and there will be no index_page set.<br>
The default values where choosen this way to have maximum security.<br>
Location-blocks have to start like this `location /path/to/look/for/ {` or like that `location .fileextension {` and have to be inside a server-block and can not overlap with another location-block.<br>
They are also closed with a single `}`.<br>
```
location /cgi/ {
		root /server/data/cgi-bin/
		method DELETE POST GET
		index_page index.cgi
	}
```

A location-block can also be set for a file-extension.<br>
In this case if any `.ico` file is requested, doesn't matter in which directory, it will always go and search for it in `server/data/images/`.<br>
```
location *.ico {
	root /server/data/images/
	method GET
	autoindex false
}
```

## error_page
This enables you to setup custom error pages for all the error pages where you don't want to show the autogenerated error page.<br>
```
`error_page 404 /server/data/pages/404.html`
```
[back to the top](https://github.com/TamLem/Webserv/blob/master/server/config/README.md#explanations-on-how-to-use-our-config-file)<br>
[back to the main README](https://github.com/TamLem/Webserv#webserv---an-http-web-server)
