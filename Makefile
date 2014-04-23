CC=gcc
CFLAGS=-lprotobuf-c -std=c99 -O2 -pedantic -Wall -I.

all: protobuf-src rtcmcap

protobuf-src:
	protoc-c --c_out=. rtcm.proto

rtcmcap: rtcmcap.o receiver.o sender.o facility.o rtcm.pb-c.o
	$(CC) -o rtcmcap rtcmcap.o receiver.o sender.o facility.o rtcm.pb-c.o $(CFLAGS)

clean:
	rm -f *.o
