#ifndef ACAMPTRANSMISSION_H
#define ACAMPTRANSMISSION_H
#include "Common.h"

extern sockaddr_in udp_addrin;

int udp_init(char* udp_addr, int udp_port);
int udp_send(int fd, u8* buf, int len);
#endif // ACAMPTRANSMISSION_H
