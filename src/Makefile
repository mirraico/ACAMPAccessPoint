LDFLAGS = -lubox -luci

all: common.o protocol.o network.o setting.o ap.o wlconf.o error.o log.o discovery.o register.o run.o main.o
	$(CC) $(LDFLAGS) common.o protocol.o network.o setting.o ap.o wlconf.o error.o log.o discovery.o register.o run.o main.o -o acamp
common.o: common.c common.h
	$(CC) $(LDFLAGS) -c common.c
protocol: protocol.c protocol.h
	$(CC) $(LDFLAGS) -c protocol.c
network: network.c network.h
	$(CC) $(LDFLAGS) -c network.c
setting: setting.c setting.h
	$(CC) $(LDFLAGS) -c setting.c
ap: ap.c ap.h
	$(CC) $(LDFLAGS) -c ap.c
wlconf: wlconf.c wlconf.h
	$(CC) $(LDFLAGS) -c wlconf.c
error: error.c error.h
	$(CC) $(LDFLAGS) -c error.c
log: log.c log.h
	$(CC) $(LDFLAGS) -c log.c
discovery: discovery.c
	$(CC) $(LDFLAGS) -c discovery.c
register: register.c
	$(CC) $(LDFLAGS) -c register.c
run: run.c
	$(CC) $(LDFLAGS) -c run.c
main: main.c
	$(CC) $(LDFLAGS) -c main.c
clean:
	rm -f *.o *.log *.tmp AP
