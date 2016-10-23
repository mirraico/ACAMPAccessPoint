CFLAGS = -g

all: protocol.o network.o ap.o error.o discovery.o main.o
	gcc protocol.o network.o ap.o error.o discovery.o main.o -o AP
protocol: protocol.c protocol.h
	gcc -c protocol.c
network: network.c network.h
	gcc -c network.c
ap: ap.c ap.h
	gcc -c ap.c
error: error.c error.h
	gcc -c error.c
discovery: discovery.c
	gcc -c discovery.c
main: main.c
	gcc -g -c main.c
clean:
	rm -f *.o AP
