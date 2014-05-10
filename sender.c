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
struct recipient *peer;
int sfd;
int nrcpnts = 0;

int transmition_init(char *recipients_list_filename)
{
    FILE * rcpnts_list = fopen(recipients_list_filename, "r");
    if (rcpnts_list == NULL) {
        p_errno(log_file, "Opening recipients list");
        return -1;
    }
    
    static struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;
    const int rcpnt_line_len = NI_MAXHOST + NI_MAXSERV + 2;
    char rcpnt_line[rcpnt_line_len];
    char * pserv;
    char * delim;
    int s;
    char hostname[NI_MAXHOST];
    char servname[NI_MAXSERV];
    size_t hostname_len;
    struct addrinfo *ai_result;
    struct addrinfo ai_list_head;
    struct addrinfo *ai_list_iter;
    ai_list_iter = &ai_list_head;
    /* resolve addresses, count and build linked list */
    while (fgets(rcpnt_line, rcpnt_line_len, rcpnts_list) != NULL)
    {
        delim = strchr(rcpnt_line, ':');
        hostname_len = delim - rcpnt_line;
        strncpy(hostname, rcpnt_line, hostname_len);
        pserv = delim + 1;
        strncpy(servname, pserv, strlen(pserv) - 1);
        fprintf(debug_file, "sender: processing recipient name %s:%s... ",
                hostname, servname);
        s = getaddrinfo(hostname, servname, &hints, &ai_result);
        if (s == 0) {
            ai_list_iter->ai_next = ai_result;
            ai_list_iter = ai_list_iter->ai_next;    
            nrcpnts++;
            fprintf(debug_file, "OK\n");
        }
        else {
            fprintf(log_file, "getaddrinfo: %s\n", gai_strerror(s));
        }
    }
    fprintf(debug_file, "sender: in sum total %d recipients found\n", nrcpnts);
    /* Allocate array and fulfill it with recipients */
    peer = calloc(nrcpnts, sizeof(struct recipient));
    memset(peer, 0, nrcpnts * sizeof(struct recipient));
    ai_list_iter = ai_list_head.ai_next;
    for (int i = 0; i < nrcpnts; i++)
    {
        memcpy(&peer[i].addr, ai_list_iter->ai_addr, ai_list_iter->ai_addrlen);
        peer[i].addr_len = ai_list_iter->ai_addrlen;
        ai_list_iter = ai_list_iter->ai_next;
    }
    freeaddrinfo(ai_list_head.ai_next);

    sfd = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
    if (sfd == -1) {
        p_errno(log_file, "Create socket");
    }
    return nrcpnts;
}

void transmition_stop()
{
    free(peer);
    close(sfd);
}

int send_within_protobuf(unsigned char *rtcm_rcv_buf, int nbytes)
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
    
    int s;
    int bytes_sent = 0;
    char host[NI_MAXHOST], service[NI_MAXSERV];
    for (int i = 0; i < nrcpnts; i++)
    {
        bytes_sent = sendto(sfd, pb_databuf, pb_datalen, 0,
                (struct sockaddr *)&peer[i].addr, peer[i].addr_len);
        if (bytes_sent == -1)
            p_errno(log_file, "sendto");
        s = getnameinfo((struct sockaddr *) &peer[i].addr,
                        peer[i].addr_len, host, NI_MAXHOST,
                        service, NI_MAXSERV,
                        NI_NUMERICHOST || NI_NUMERICSERV);
        if (s == 0)
            fprintf(debug_file, "sender: %ld bytes sent to %s:%s UDP\n",
                        (long)bytes_sent, host, service);
        else
            fprintf(log_file, "getnameinfo: %s\n", gai_strerror(s));
    }
    free(pb_rtcm_msg.msg.data);
    pb_rtcm_msg.msg.data = NULL;
    free(pb_databuf);
    pb_databuf = NULL;
    return bytes_sent;
}
