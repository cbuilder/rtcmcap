#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>

#define BAUDRATE 19200
#define RTCMDEVICE "/dev/ttyS2"

char buf[255];

int rcvr_port_open() {
	int fd,c, res;
        fd = open(RTCMDEVICE, O_RDONLY | O_NOCTTY );
	if (fd <0) {
		perror(RTCMDEVICE);
		exit(-1);
	}
	return fd;
}

int main()
{
	int rtcm_fd = rcvr_port_open();
	return 0;
}
