/*
 * File: receiver.h
 * Description: Serial connection and message receive function prototypes
 */

int rcvr_port_open();
int rcvr_port_close(int fd);
int receive_rtcm(unsigned char *buf, int fd);
