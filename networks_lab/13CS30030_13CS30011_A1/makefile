all: server.c reporter.c
	gcc server.c -o server -pthread -std=c99 `mysql_config --cflags --libs` -g
	gcc reporter.c -o reporter
	gcc reader.c -o reader
	gcc admin.c -o admin -lcrypt

reporter: reporter.c
	gcc reporter.c -o reporter

server: server.c
	gcc server.c -o server -pthread -std=c99 `mysql_config --cflags --libs`

reader: reader.c
	gcc reader.c -o reader

admin: admin.c
	gcc admin.c -o admin

clean:
	rm -rf *.o
