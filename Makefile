all: AcampProtocol.o APMain.o
	gcc AcampProtocol.o APMain.o -o ACAMPAccessPoint
AcampProtocol: AcampProtocol.c AcampProtocol.h
	gcc -c AcampProtocol.c
APMain: APMain.c
	gcc -c APMain.c
clean:
	rm -f *.o ACAMPAccessPoint
