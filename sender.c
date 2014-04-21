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
#include <errno.h>
#include <string.h>
#include "rtcmcap.h"
#include "facility.h"

extern FILE * log_file, *debug_file;
static struct sockaddr_in claddr;

int sock_create()
{
    int sfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); // SOCK_RAW
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

int send_to_client(int sfd, unsigned char *buf, int bytes)
{
    int bytes_sent = sendto(sfd, buf, bytes, 0,
                (struct sockaddr *)&claddr, sizeof(claddr));
    fprintf(debug_file, "Sender: Bytes sent: %d\n", bytes_sent);
    if (bytes_sent == -1)
        p_errno(log_file, "sendto");
    return bytes_sent;
}
