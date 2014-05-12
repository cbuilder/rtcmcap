/*
 * File: rtcmcap.c
 * Description: Main project file
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include "rtcmcap.h"
#include "receiver.h"
#include "sender.h"

FILE *log_file;
FILE *debug_file;
volatile unsigned char stop = 0;

void p_current_time(FILE *fd)
{
    time_t cur_time;
    time(&cur_time);
    char time_string[16];
    snprintf(time_string, 16, "%s", ctime(&cur_time) + 4);
    fprintf(fd, "%s  ", time_string);
}

void p_errno(FILE *fd, const char *format, ...)
{
    if (fd != stdout)
        p_current_time(fd);
    char *errmsg = strerror(errno);
    va_list arglist;
    va_start(arglist, format);
    vfprintf(fd, format, arglist);
    va_end(arglist);
    fprintf(fd, ": %s\n", errmsg);
}

void p_msg(FILE *fd, const char *format, ...)
{
    if (fd != stdout)
        p_current_time(fd);
    va_list arglist;
    va_start(arglist, format);
    vfprintf(fd, format, arglist);
    va_end(arglist);
}

static void exit_condition_handler()
{
    p_msg(log_file, "Stopping rtcmcap.\n");
    stop = 1;
}

int main()
{
#if defined DEBUG
    log_file = stdout;
    debug_file = stdout;
#elif defined DEBUG_TO_LOG
    log_file = fopen(LOG_FILE, "w");
    if (log_file == NULL)
        p_errno(stdout, "log file opening");
    debug_file = log_file;
#else
    log_file = fopen(LOG_FILE, "w");
    if (log_file == NULL)
        p_errno(stdout, "log file opening");
    debug_file = fopen("/dev/null", "w");
#endif
    int nbytes;
    unsigned char rtcm_rcv_buf[RCV_BUF_SZ];
  
    p_msg(log_file, "Starting RTCM capture program.\n");

    if (signal(SIGINT, exit_condition_handler) == SIG_ERR)
        p_errno(log_file, "Redirect terminal interrupt signal");

    int rtcm_stream_fd = rcvr_port_open();
    if (rtcm_stream_fd == -1) 
        exit(EXIT_FAILURE);

    int recipients_number = transmition_init(RECIPIENTS_LIST);
    if (recipients_number <= 0)
        exit(EXIT_FAILURE);

    while(!stop) {
        nbytes = receive_rtcm(rtcm_rcv_buf, rtcm_stream_fd);
        if (nbytes > 0)
            send_within_protobuf(rtcm_rcv_buf, nbytes);
    }

    rcvr_port_close(rtcm_stream_fd);
    transmition_stop();
    exit(EXIT_SUCCESS);
}
