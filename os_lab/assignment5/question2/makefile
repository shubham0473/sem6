all: manager train ass5q2.h

manager: manager.c ass5q2.h
	gcc manager.c -o manager -std=gnu99 -pthread

train: train.c ass5q2.h
	gcc train.c -o train -std=gnu99 -pthread

clean:
	rm manager train
