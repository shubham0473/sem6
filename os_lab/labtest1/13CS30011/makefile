all: master.out node.out

master.out: 13CS30011_master.c
	gcc 13CS30011_master.c -o master.out -std=gnu99

node.out: 13CS30011_node.c
	gcc 13CS30011_node.c -o node.out -std=gnu99

clean:
	rm -rf *.out
