all: AcampAP.o AcampProtocol.o AcampTransmission.o main.o
	g++ AcampAP.o AcampProtocol.o AcampTransmission.o main.o -o ACAMPAccessPoint
AcampAP: AcampAP.cpp AcampAP.h
	g++ -c AcampAP.cpp
AcampProtocol: AcampProtocol.cpp AcampProtocol.h
	g++ -c AcampProtocol.cpp
AcampTransmission: AcampTransmission.cpp AcampTransmission.h
	g++ -c AcampTransmission.cpp
main: main.cpp
	g++ -c main.cpp
clean:
	rm -f *.o ACAMPAccessPoint
