/*
 * File: sender.h
 * Description: Network function prototypes
 */

#ifndef NI_MAXHOST
#define NI_MAXHOST    1025
#endif
#ifndef NI_MAXSERV
#define NI_MAXSERV      32
#endif

int sock_create();
void sock_close();
int send_within_protobuf(int socket_fd, unsigned char *buf, size_t num_bytes);
