/*
 * File: sender.h
 * Description: Network function prototypes
 */

int sock_create();
void sock_close();
int send_to_client(int socket_fd, unsigned char *buf, size_t num_bytes);
