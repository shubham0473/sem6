all: server client

server: server.c
	gcc server.c -o server -std=gnu99

client: client.c
	gcc client.c -o client -std=gnu99

clean:
	rm server client *~
