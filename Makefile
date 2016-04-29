all: AcampAP.o AcampNetwork.o AcampProtocol.o APMain.o
	g++ AcampAP.o AcampNetwork.o AcampProtocol.o APMain.o -o ACAMPAccessPoint
AcampAP: AcampAP.cpp AcampAP.h
	g++ -c AcampAP.cpp
AcampNetwork: AcampNetwork.cpp AcampNetwork.h
	g++ -c AcampNetwork.cpp
AcampProtocol: AcampProtocol.cpp AcampProtocol.h
	g++ -c AcampProtocol.cpp
APMain: APMain.cpp
	g++ -c APMain.cpp
clean:
	rm -f *.o ACAMPAccessPoint
