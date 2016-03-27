all: master atm client

master: master.cpp
	g++ master.cpp -o master -lrt -lpthread -g
	sudo chown -v root:root master
	sudo chmod -v 4755 master

atm: atm.cpp
	g++ atm.cpp -o atm -lrt -lpthread -g
	sudo chown -v root:root atm
	sudo chmod -v 4755 atm

client: client.cpp
	g++ client.cpp -o client -lrt -lpthread -g
	sudo chown -v root:root client
	sudo chmod -v 4755 client

clean:
	rm master atm client
