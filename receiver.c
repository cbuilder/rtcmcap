/*
 * File: receiver.c
 * Description: Serial connection and message receive functions
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <unistd.h>
#include "rtcmcap.h"

extern FILE * log_file, *debug_file;
static struct termios term_settings, orig_term_settings;

static const struct itimerval pream_rcv_iti = {
        .it_interval = { 0, 0 },
        .it_value = {2, 0},
};
static const struct itimerval msg_rcv_iti = {
        .it_interval = { 0, 0 },
        .it_value = {0, 500},
};
static const struct itimerval disable_iti = {
        .it_interval = { 0, 0 },
        .it_value = {0, 0},
};

int rcvr_port_open()
{
    int fd;
    fd = open(RTCMDEVICE, O_RDONLY | O_NOCTTY );
    if (fd == -1)
        p_errno(log_file, "Open %s", RTCMDEVICE);
    if (tcgetattr(fd, &orig_term_settings) == -1)
        p_errno(log_file, "Get terminal settings of %s", RTCMDEVICE);
    memset(&term_settings, 0, sizeof(term_settings));
    term_settings.c_cflag = BRATE | CS8 | CLOCAL | CREAD;
    term_settings.c_iflag = IGNPAR;
    term_settings.c_oflag = 0;
    term_settings.c_lflag = 0;
    term_settings.c_cc[VTIME] = 0;
    term_settings.c_cc[VMIN] = 1;
    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &term_settings) == -1)
        p_errno(log_file, "Set terminal settings for %s", RTCMDEVICE);
    return fd;
}

int rcvr_port_close(int fd)
{
    if (tcsetattr(fd, TCSANOW, &orig_term_settings) == -1)
        p_errno(log_file, "Return terminal settings for %s", RTCMDEVICE);
    return close(fd);
}

static void line_activity_timeout_handler()
{
    p_msg(log_file, "The receiver on %s keeps calm\n", RTCMDEVICE);
    sleep(5);
}

static void msg_rcv_timeout_handler()
{
    ;
}

int receive_rtcm(unsigned char *buf, int fd)
{
    struct itimerval curr_iti;
    unsigned int nread = 0;
    unsigned int msg_length = 0;
    memset(buf, 0, sizeof(buf));
    if (signal(SIGALRM, line_activity_timeout_handler) == SIG_ERR)
        p_errno(log_file, "Signal for message wait timeout");
    if (setitimer(ITIMER_REAL, &pream_rcv_iti, NULL) == -1)
        p_errno(log_file, "Start timer for message wait");
    nread = read(fd, &buf[0], 1);
    if (nread == -1) {
        if (errno == EINTR)
            ; /* Timeout or exit */
        else
            p_errno(log_file, "Read from %s", RTCMDEVICE);
    }
    else if (nread == 0)
        p_msg(log_file, "Achtung! Keine bytes bekommt aus %s!", RTCMDEVICE);
    else {
        getitimer(ITIMER_REAL, &curr_iti);
        unsigned long usec_elapsed_from_pream_rcv =
            ((unsigned long)pream_rcv_iti.it_value.tv_sec * 1000000
             + (unsigned long)pream_rcv_iti.it_value.tv_usec)
            - ((unsigned long)curr_iti.it_value.tv_sec * 1000000
               + (unsigned long)curr_iti.it_value.tv_usec);
        p_msg(debug_file, "receiver: %ld usec elapsed from last message\n",
                usec_elapsed_from_pream_rcv);
        if (setitimer(ITIMER_REAL, &disable_iti, NULL) == -1)
            p_errno(log_file, "Stop itimer");
        if (usec_elapsed_from_pream_rcv < 500)
            p_msg(debug_file, "receiver: waiting more for the beginning\n");
        else {
            if (buf[0] == RTCM2_PREAMBLE || buf[0] == UNKNOWN_PREAMBLE) {
                unsigned int i = 1;
                while (1) { /* Message receive cycle */
                    if (signal(SIGALRM, msg_rcv_timeout_handler) == SIG_ERR)
                        p_errno(log_file, "Signal for byte wait timeout");
                    if (setitimer(ITIMER_REAL, &msg_rcv_iti, NULL) == -1)
                        p_errno(log_file, "Start timer for byte wait");
                    nread = read(fd, &buf[i], 1);
                    if (setitimer(ITIMER_REAL, &disable_iti, NULL) == -1)
                        p_errno(log_file, "Stop itimer");

                    if (nread == -1 ) {
                        if (errno == EINTR)
                            break; /* Timed out: message end */
                        else
                            p_errno(log_file, "Read from %s", RTCMDEVICE);
                    }
                    else if (nread == 0) {
                        p_msg(log_file, "Read from %s reached EOF\n",
                                RTCMDEVICE);
                        break;
                    }
                    else
                        i++; /* Continue to receive message */
                } /* End of message receive cycle */
                msg_length = i;
#ifndef DEBUG
                p_msg(debug_file, "receiver: got message ");
                for (unsigned j = 0; j < msg_length; j++) {
                    if (j > 0) fprintf(debug_file, ",");
                    fprintf(debug_file, "%02X", buf[j]);
                }
                fprintf(debug_file, "; length: %u\n", msg_length);
#endif
            }
        }
    }
    return msg_length;
}

