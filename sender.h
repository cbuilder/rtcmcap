/*
 * File: sender.h
 * Description: Network function prototypes
 */

int sock_create();
void sock_close();
int send_within_protobuf(int socket_fd, unsigned char *buf, size_t num_bytes);
