CFLAGS = -g

all: protocol.o network.o setting.o ap.o error.o log.o discovery.o main.o
	gcc protocol.o network.o setting.o ap.o error.o log.o discovery.o main.o -o AP
protocol: protocol.c protocol.h
	gcc -c protocol.c
network: network.c network.h
	gcc -c network.c
setting: setting.c setting.h
	gcc -c setting.c
ap: ap.c ap.h
	gcc -c ap.c
error: error.c error.h
	gcc -c error.c
log: log.c log.h
	gcc -c log.c
discovery: discovery.c
	gcc -c discovery.c
main: main.c
	gcc -g -c main.c
clean:
	rm -f *.o *.log AP
