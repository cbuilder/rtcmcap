CC=gcc
CFLAGS= -O2 -I.

rtcmcap: rtcmcap.o
	$(CC) -o rtcmcap rtcmcap.o $(CFLAGS)

clean:
	rm -f *.o
