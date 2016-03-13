all: manager producer consumer unit_test evaluate ass5.h

evaluate: evaluate.c manager
	gcc evaluate.c -o evaluate -std=gnu99

manager: manager.c ass5.h producer consumer
	gcc manager.c -o manager -std=gnu99 -pthread

producer: producer.c ass5.h
	gcc producer.c -o producer -std=gnu99 -pthread

consumer: consumer.c ass5.h
	gcc consumer.c -o consumer -std=gnu99 -pthread

unit_test: unit_test.c ass5.h
	gcc unit_test.c -o unit_test -std=gnu99

clean:
	rm manager consumer producer unit_test
