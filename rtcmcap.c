#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <unistd.h>

#define DEBUG
#define RATE		19200
#define BRATE		B##19200
#define RTCMDEVICE	"/dev/ttyS2"
#define RTCM2_PREAMBLE	0x66
#define UNKNOWN_PREAMBLE	0x59
#define FRAME_BUF_SZ	256
#define MAX_T_BETW_BYTES_IN_MSG (1/RATE * 3)*1000000 /* usec */

volatile unsigned char stop = 0;
struct termios term_settings, orig_term_settings;

int rcvr_port_open()
{
    int fd;
    fd = open(RTCMDEVICE, O_RDONLY | O_NOCTTY );
    if (fd <0) {
        perror(RTCMDEVICE);
        exit(-1);
    }
    tcgetattr(fd, &orig_term_settings);
    memset(&term_settings, 0, sizeof(term_settings));
    term_settings.c_cflag = BRATE | CS8 | CLOCAL | CREAD;
    term_settings.c_iflag = IGNPAR;
    term_settings.c_oflag = 0;
    term_settings.c_lflag = 0;

    term_settings.c_cc[VTIME] = 10; /* Port timeout 1 second */
    term_settings.c_cc[VMIN] = 1;

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &term_settings);
    return fd;
}

int rcvr_port_close(int fd)
{
    tcsetattr(fd, TCSANOW, &orig_term_settings);
    return close(fd);
}

void exit_handler()
{
    puts("Stopping capture.");
    stop = 1;
}

void line_activity_timeout_handler()
{
    puts("The receiver is keeping silence");
}

void msg_rcv_timeout_handler()
{
    ;
}

unsigned char * receive(unsigned char *buf, int fd)
{
    struct itimerval curr_iti;
    const struct itimerval pream_rcv_iti = {
        .it_interval = { 0, 0 },
        .it_value = {2, 0}, /* Max time between messages */
    };
    const struct itimerval msg_rcv_iti = {
        .it_interval = { 0, 0 },
        .it_value = {0, 500},
    };
    const struct itimerval disable_iti = {
        .it_interval = { 0, 0 },
        .it_value = {0, 0},
    };
    unsigned int rcvd_msg_len = 0;
    unsigned int nread = 0;
    memset(buf, 0, sizeof(buf));

    if (signal(SIGALRM, line_activity_timeout_handler) == SIG_ERR)
        perror("signal to wait preamble");
    if (setitimer(ITIMER_REAL, &pream_rcv_iti, NULL) == -1)
        perror("setitimer on receive preamble");
    nread = read(fd, &buf[0], 1);
    getitimer(ITIMER_REAL, &curr_iti);
    unsigned long usec_elapsed_from_pream_rcv =
            ((unsigned long)pream_rcv_iti.it_value.tv_sec * 1000000
            + (unsigned long)pream_rcv_iti.it_value.tv_usec)
            - ((unsigned long)curr_iti.it_value.tv_sec * 1000000
            + (unsigned long)curr_iti.it_value.tv_usec);
#ifdef DEBUG
    printf("usec elapsed from last msg: %ld\n", usec_elapsed_from_pream_rcv);
#endif
    if (setitimer(ITIMER_REAL, &disable_iti, NULL) == -1)
        perror("setitimer to stop");

    if (nread > 0 && usec_elapsed_from_pream_rcv > 500) {
        if (buf[0] == RTCM2_PREAMBLE || buf[0] == UNKNOWN_PREAMBLE) {
            unsigned int i = 1;
            while (1) {
                if (signal(SIGALRM, msg_rcv_timeout_handler) == SIG_ERR)
                    perror("signal");
                if (setitimer(ITIMER_REAL, &msg_rcv_iti, NULL) == -1)
                    perror("setitimer on receive msg");
                nread = read(fd, &buf[i], 1);
                if (setitimer(ITIMER_REAL, &disable_iti, NULL) == -1)
                    perror("setitimer to stop");
                if (nread == -1 ) {
                    if (errno == EINTR) {
                        break; /* Message read timed out */
                    } else {
                        ; /* Unknown error */
                    }
                } else if (nread == 0) {
                    break; /* EOF */
                } else {
                    i++;
                }
            }
            rcvd_msg_len = i;
#ifdef DEBUG
            printf("Received Message:");
            for (unsigned j = 0; j < rcvd_msg_len; j++) {
                if (j > 0) printf(",");
                printf("%02X", buf[j]);
            }
            printf("\nLength: %u\n",rcvd_msg_len);
#endif
        }
    } else if (nread == 0) {
        printf("Receiver on %s is not alive", RTCMDEVICE);
        rcvr_port_close(fd);
        exit(-1);
    } else {
        ; //Error
    }
    return buf;
}

int main()
{
    unsigned char rtcm_frame_buf[FRAME_BUF_SZ];
    unsigned char *rtcm_msg;
    if (signal(SIGINT, exit_handler) == SIG_ERR)
        perror("terminal interrupt signal");
    int rtcm_stream_fd = rcvr_port_open();
    while(!stop) {
        rtcm_msg = receive(rtcm_frame_buf, rtcm_stream_fd);
    }
    rcvr_port_close(rtcm_stream_fd);
    exit(EXIT_SUCCESS);
}
