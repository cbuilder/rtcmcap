#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>

#define BAUDRATE 19200
#define RTCMDEVICE "/dev/ttyS2"

struct termios term_settings, orig_term_settings;

int rcvr_port_open() {
	int fd,c, res;
        fd = open(RTCMDEVICE, O_RDONLY | O_NOCTTY );
	if (fd <0) {
		perror(RTCMDEVICE);
		exit(-1);
	}
	tcgetattr(fd, &orig_term_settings);
	memset(&term_settings, 0, sizeof(term_settings));
	term_settings.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	term_settings.c_iflag = IGNPAR;
	term_settings.c_oflag = 0;
	term_settings.c_lflag = 0;
	
	term_settings.c_cc[VTIME] = 0;
	term_settings.c_cc[VMIN] = 1;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &term_settings);
	return fd;
}

int main()
{
	char input;
	int rtcm_stream_fd = rcvr_port_open();
	while (1) {
		read(rtcm_stream_fd, &input, 1);
	}
	return 0;
}
