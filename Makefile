#CFLAGS = -g

all: protocol.o network.o ap.o main.o
	gcc protocol.o network.o ap.o main.o -o ACAMPAccessPoint
protocol: protocol.c protocol.h
	gcc -c protocol.c
network: network.c network.h
	gcc -c network.c
ap: ap.c ap.h
	gcc -c ap.c
main: main.c
	gcc -g -c main.c
clean:
	rm -f *.o ACAMPAccessPoint
