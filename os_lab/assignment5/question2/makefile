all: manager train ass5q2.h

manager: manager.c ass5q2.h
	gcc manager.c -o manager -std=gnu99 -lrt -lpthread -g
	sudo chown -v root:root manager
	sudo chmod -v 4755 manager

train: train.c ass5q2.h
	gcc train.c -o train -std=gnu99 -lrt -lpthread -g
	sudo chown -v root:root train
	sudo chmod -v 4755 train

clean:
	rm manager train
