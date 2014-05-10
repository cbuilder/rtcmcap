/*
 * File: rcmcap.h
 * Description: Main project definitions 
 */

#define RATE		19200
#define BRATE		B##19200
#define RTCMDEVICE	"/dev/ttyS2"
#define RTCM2_PREAMBLE	0x66
#define UNKNOWN_PREAMBLE	0x59
#define RCV_BUF_SZ	256
#define CLIENT_IPADDR "192.168.2.1"
#define CLIENT_PORT "12345"
#define RECIPIENTS_LIST "/etc/rtcmcap/recipients.list"
