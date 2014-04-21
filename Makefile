CC=gcc
CFLAGS= -ggdb -std=c99 -O2 -pedantic -Wall -I.

rtcmcap: rtcmcap.o receiver.o sender.o facility.o
	$(CC) -o rtcmcap rtcmcap.o receiver.o sender.o facility.o $(CFLAGS)

clean:
	rm -f *.o
