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
    int udp_socket_fd = sock_create();
    if (udp_socket_fd == -1)
        exit(EXIT_FAILURE);

    while(!stop) {
        nbytes = receive_rtcm(rtcm_rcv_buf, rtcm_stream_fd);
        if (nbytes > 0)
            nbytes = send_within_protobuf(udp_socket_fd,
                                            rtcm_rcv_buf, nbytes);
    }

    rcvr_port_close(rtcm_stream_fd);
    sock_close(udp_socket_fd);
    exit(EXIT_SUCCESS);
}
