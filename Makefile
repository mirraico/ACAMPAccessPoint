all: protocol.o network.o main.o
	gcc protocol.o network.o main.o -o ACAMPAccessPoint
protocol: protocol.c protocol.h
	gcc -c protocol.c
network: network.c network.h
	gcc -c network.c
main: main.c
	gcc -c main.c
clean:
	rm -f *.o ACAMPAccessPoint
