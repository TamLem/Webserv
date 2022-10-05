all:
	bash init.sh
	c++ curl_example.cpp -o ../testserv -lcurl

clean:
	bash revert.sh
	-rm ../testserv