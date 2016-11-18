CFLAGS = -g

all: protocol.o common.o main.o
	gcc protocol.o common.o main.o -o AC
common.o: common.c common.h
	gcc -c common.c
protocol.o: protocol.c protocol.h
	gcc -c protocol.c
main: main.c
	gcc -g -c main.c
clean:
	rm -f *.o *.log AC
