all: ping.out troute.out

ping.out: ping.c
	gcc ping.c -o ping.out -std=gnu99 -lm
	sudo chown -v root:root ping.out
	sudo chmod -v 4755 ping.out
