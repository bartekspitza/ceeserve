#include <netinet/in.h>
#ifndef STATIC_H
#define STATIC_H

void handle_conn(int socket, struct sockaddr_in cilent_addr);
void error(const char *msg);

#endif