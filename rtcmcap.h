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
#define RECIPIENTS_LIST "/etc/rtcmcap/recipients.list"
#define LOG_FILE "/var/log/rtcmcap/rtcmcap.log"

void p_current_time(FILE *fd);
void p_errno(FILE *fd, const char *format, ...);
void p_msg(FILE *fd, const char *format, ...);
