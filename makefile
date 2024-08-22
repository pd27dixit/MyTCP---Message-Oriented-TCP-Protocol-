CC=gcc
CFLAGS=-lpthread

all: libmysock.a

mysocket.o: mysocket.c
	$(CC) -c mysocket.c -o mysocket.o $(CFLAGS)

libmysock.a: mysocket.o
	ar rcs libmysock.a mysocket.o

clean:
	rm -f mysocket.o libmysock.a

