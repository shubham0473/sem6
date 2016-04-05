all: master.out node.out

master.out: master.c
	gcc master.c -o master.out -std=gnu99

node.out: node.c
	gcc node.c -o node.out -std=gnu99

clean:
	rm -rf *.out
