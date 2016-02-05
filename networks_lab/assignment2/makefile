all: server.c
	gcc server.c -o server -std=c99 `mysql_config --cflags --libs` -g
	gcc client.c -o client -g


server: server.c
	gcc server.c -o server -std=c99 `mysql_config --cflags --libs`

client: client.c
	gcc client.c -o client -std=c99

clean:
	rm -rf *.o
