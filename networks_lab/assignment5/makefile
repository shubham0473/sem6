all: tracert.out troute.out

tracert.out: tracert.c
	gcc tracert.c -o tracert.out -std=gnu99 -lm
	sudo chown -v root:root tracert.out
	sudo chmod -v 4755 tracert.out

troute.out: troute.c
	gcc troute.c -o troute.out -std=gnu99
	sudo chown -v root:root troute.out
	sudo chmod -v 4755 troute.out

clean:
	rm tracert.out troute.out
