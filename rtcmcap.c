/*
 * File: rtcmcap.c
 * Description: Main project file
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "rtcmcap.h"
#include "receiver.h"
#include "sender.h"
#include "facility.h"

FILE *log_file;
FILE *debug_file;
volatile unsigned char stop = 0;

static void exit_condition_handler()
{
    fprintf(log_file, "Stopping RTCM capture program.\n");
    stop = 1;
}

int main()
{
    log_file = stdout;
    debug_file = stdout;
    int nbytes;
    unsigned char rtcm_rcv_buf[RCV_BUF_SZ];

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
