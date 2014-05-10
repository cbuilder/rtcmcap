/*
 * File: sender.h
 * Description: Network function prototypes
 */

int transmition_init(char *recipients_list_filename);
void transmition_stop();
int send_within_protobuf(unsigned char *buf, size_t num_bytes);
