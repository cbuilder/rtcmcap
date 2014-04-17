CC=gcc
CFLAGS= -std=c99 -O2 -pedantic -Wall -I.

rtcmcap: rtcmcap.o
	$(CC) -o rtcmcap rtcmcap.o $(CFLAGS)

clean:
	rm -f *.o
