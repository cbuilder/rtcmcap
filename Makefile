CC=gcc
CFLAGS=-lprotobuf-c -ggdb -std=c99 -O2 -pedantic -Wall -I.

all: rtcmcap

rtcmcap: rtcmcap.o receiver.o sender.o facility.o rtcm.pb-c.o
	$(CC) -o rtcmcap rtcmcap.o receiver.o sender.o facility.o rtcm.pb-c.o $(CFLAGS)

clean:
	rm -f *.o
