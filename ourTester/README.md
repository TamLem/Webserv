# Tester

You can use this tester through the makefile of our webserv project.<br>

With `make tester` you can initialize the filesystem permisions for testing (it runs the [init.sh](https://github.com/TamLem/Webserv/blob/master/ourTester/init.sh)), create the tester executable `testserv`, makes and starts our webserv with corresponding config file.<br>

⚠️ AFTER TESTING USE `make rmtester`, otherwise some of the filesystem can't be accessed anymore.⚠️<br>
`make rmtester` will cleanup by reverting filesystem permission and removes tester executable `testerv` after you are done testing (it runs the [revert.sh](https://github.com/TamLem/Webserv/blob/master/ourTester/revert.sh)).<br>


In the output of the tester you can see which test comes from which line of code so it is easier to find out which test resulted in what.<br>
These tests could be extended as far as you like, since it is quite self-explanatory.<br>

[back to the main README](https://github.com/TamLem/Webserv#webserv---an-http-web-server)
