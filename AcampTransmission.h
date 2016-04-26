#ifndef __ACAMPTRANSMISSION_H__
#define __ACAMPTRANSMISSION_H__

extern int udp_fd;
extern sockaddr_in udp_addrin;

void udp_init(char* udp_addr, int udp_port);
int udp_send(u8* buf, int len);
int udp_recv(u8* buf, int len);
#endif // ACAMPTRANSMISSION_H
