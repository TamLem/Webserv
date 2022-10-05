Use through makefile of webserv project.

```make tester``` (initializes filesystem permisions for testing, creates tester executable "testserv" and makes webserv with corresponding config)

```make rmtester``` to cleanup (reverting filesystem permission changes and removes tester executable "testerv")

to execute: ```./testserv```

AFTER TESTING USE ```make rmtester```, otherwise some of the filesystem can't be added to git