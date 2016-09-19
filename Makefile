all: AcampProtocol.o AcampNetwork.o APMain.o
	gcc AcampProtocol.o AcampNetwork.o APMain.o -o ACAMPAccessPoint
AcampProtocol: AcampProtocol.c AcampProtocol.h
	gcc -c AcampProtocol.c
AcampNetwork: AcampNetwork.c AcampNetwork.h
	gcc -c AcampNetwork.c
APMain: APMain.c
	gcc -c APMain.c
clean:
	rm -f *.o ACAMPAccessPoint
