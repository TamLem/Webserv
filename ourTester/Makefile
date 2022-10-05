all:
	bash init.sh
	c++ curl_example.cpp -o ../testserv -lcurl
	make our -C ..

clean:
	bash revert.sh
	-rm ../testserv