CFLAGS = -g

all: common.o protocol.o network.o setting.o ap.o error.o log.o discovery.o register.o configure.o main.o
	gcc common.o protocol.o network.o setting.o ap.o error.o log.o discovery.o register.o configure.o main.o -o AP
common.o: common.c common.h
	gcc -c common.c
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
register: register.c
	gcc -c register.c
configure: configure.c
	gcc -c configure.c
main: main.c
	gcc -g -c main.c
clean:
	rm -f *.o *.log AP
