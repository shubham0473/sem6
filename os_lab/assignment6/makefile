all: master atm client

master: master.c
	gcc master.c -o master -std=gnu99 -lrt -lpthread -g
	# sudo chown -v root:root master
	# sudo chmod -v 4755 master

atm: atm.c
	gcc atm.c -o atm -std=gnu99 -lrt -lpthread -g
	# sudo chown -v root:root atm
	# sudo chmod -v 4755 atm

client: client.c
	gcc client.c -o client -std=gnu99 -lrt -lpthread -g
	# sudo chown -v root:root client
	# sudo chmod -v 4755 client

clean:
	rm master atm
