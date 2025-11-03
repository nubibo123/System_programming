CC=gcc
CFLAGS=-Wall -g

all: producer consumer

producer: producer.c buffer.h
	$(CC) $(CFLAGS) -o producer producer.c

consumer: consumer.c buffer.h
	$(CC) $(CFLAGS) -o consumer consumer.c

clean:
	rm -f producer consumer *.o
