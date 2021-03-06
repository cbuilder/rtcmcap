CC=gcc
CFLAGS=-std=c99 -D_POSIX_SOURCE -O2 -pedantic -Wall -I.
LDFLAGS=-lprotobuf-c

all: rtcmcap

rtcmcap: rtcmcap.o receiver.o sender.o rtcm.pb-c.o
	$(CC) -o rtcmcap rtcmcap.o receiver.o sender.o rtcm.pb-c.o $(CFLAGS) $(LDFLAGS)

rtcmcap.o: rtcmcap.c rtcmcap.h receiver.h sender.h
	$(CC) -c rtcmcap.c $(CFLAGS)

receiver.o: receiver.c rtcmcap.h
	$(CC) -c receiver.c $(CFLAGS)

sender.o: sender.c rtcmcap.h rtcm.pb-c.h
	$(CC) -c sender.c $(CFLAGS)

rtcm.pb-c.h: rtcm.proto
	protoc-c --c_out=. rtcm.proto

rtcm.pb-c.o: rtcm.pb-c.c rtcm.pb-c.h
	$(CC) -c rtcm.pb-c.c $(CFLAGS)

clean:
	rm -f *.o rtcm.pb-c.* rtcmcap
