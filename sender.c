/*
 * File: sender.c
 * Description: Network functions
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "rtcmcap.h"
#include "facility.h"
#include "rtcm.pb-c.h"

#ifndef NI_MAXHOST
#define NI_MAXHOST    1025
#endif
#ifndef NI_MAXSERV
#define NI_MAXSERV      32
#endif

struct recipient {
     struct sockaddr_storage addr;
     socklen_t addr_len;
};

extern FILE * log_file, *debug_file;
struct recipient peer;

int sock_create()
{
    static struct addrinfo hints;
    struct addrinfo *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;
// for:
    int s = getaddrinfo(CLIENT_IPADDR, CLIENT_PORT, &hints, &result);
    if (s != 0) {
        fprintf(log_file, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        memset(&peer, 0, sizeof(struct recipient));
        memcpy(&peer.addr, rp->ai_addr, rp->ai_addrlen);
        peer.addr_len = rp->ai_addrlen;
    }
    freeaddrinfo(result);
    
    int sfd = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
    if (sfd == -1) {
        p_errno(log_file, "Create socket");
    }
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
    fprintf(debug_file, "sender: %d bytes message in protobuf\n", pb_datalen);
    
    int bytes_sent = sendto(sfd, pb_databuf, pb_datalen, 0,
                (struct sockaddr *)&peer.addr, peer.addr_len);
    if (bytes_sent == -1)
        p_errno(log_file, "sendto");
 
    char host[NI_MAXHOST], service[NI_MAXSERV];
    int s = getnameinfo((struct sockaddr *) &peer.addr,
                        peer.addr_len, host, NI_MAXHOST,
                        service, NI_MAXSERV,
                        NI_NUMERICHOST || NI_NUMERICSERV);
    if (s == 0)
        fprintf(debug_file, "sender: %ld bytes sent to %s:%s UDP\n",
                        (long)bytes_sent, host, service);
    else
        fprintf(log_file, "getnameinfo: %s\n", gai_strerror(s));

    free(pb_rtcm_msg.msg.data);
    pb_rtcm_msg.msg.data = NULL;
    free(pb_databuf);
    pb_databuf = NULL;
    
    return bytes_sent;
}
