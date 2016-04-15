all: AcampProtocol.o AcampTransmission.o main.o
	g++ AcampProtocol.o AcampTransmission.o main.o -o ACAMPAccessPoint
AcampProtocol: AcampProtocol.cpp AcampProtocol.h
	g++ -c AcampProtocol.cpp
AcampTransmission: AcampTransmission.cpp AcampTransmission.h
	g++ -c AcampTransmission.cpp
main: main.cpp
	g++ -c main.cpp
clean:
	rm -f *.o ACAMPAccessPoint
