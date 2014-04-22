/*
 * File: sender.c
 * Description: Network functions
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "rtcmcap.h"
#include "facility.h"
#include "rtcm.pb-c.h"

extern FILE * log_file, *debug_file;
static struct sockaddr_in claddr;

int sock_create()
{
    int sfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sfd == -1)
        p_errno(log_file, "Create socket");
    memset(&claddr, 0, sizeof(claddr));
    claddr.sin_family = AF_INET;
    claddr.sin_addr.s_addr = inet_addr(CLIENT_IPADDR);
    claddr.sin_port = htons(CLIENT_PORT);
    return sfd;
}

void sock_close(int sfd)
{
    close(sfd);
}

int send_within_protobuf(int sfd, unsigned char *rtcm_rcv_buf, int nbytes)
{
    unsigned int pb_datalen;
    void *pb_databuf;
    Rtcm2Message pb_rtcm_msg = RTCM2_MESSAGE__INIT;
    pb_rtcm_msg.msg.data = malloc(nbytes);
    if (pb_rtcm_msg.msg.data == NULL) {
        p_errno(log_file, "Malloc");
        return 0;
    }  
    memcpy(pb_rtcm_msg.msg.data, rtcm_rcv_buf, nbytes);
    pb_rtcm_msg.msg.len = nbytes;
    pb_datalen = rtcm2_message__get_packed_size(&pb_rtcm_msg);
    pb_databuf = malloc(pb_datalen);
    if (pb_databuf == NULL) {
        p_errno(log_file, "Malloc");
        return 0;
    }  
    rtcm2_message__pack(&pb_rtcm_msg, pb_databuf);
    
    int bytes_sent = sendto(sfd, pb_databuf, pb_datalen, 0,
                (struct sockaddr *)&claddr, sizeof(claddr));
 
    free(pb_rtcm_msg.msg.data);
    pb_rtcm_msg.msg.data = NULL;
    free(pb_databuf);
    pb_databuf = NULL;

    fprintf(debug_file, "sender: %d bytes wrapped to protobuffer, %d sent\n",
            nbytes, bytes_sent);
    if (bytes_sent == -1)
        p_errno(log_file, "sendto");
    return bytes_sent;
}
