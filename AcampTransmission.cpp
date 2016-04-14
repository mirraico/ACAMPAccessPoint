#include "AcampTransmission.h"

sockaddr_in udp_addrin;
int udp_fd;

void udp_init(char* udp_addr, int udp_port)
{
    udp_addrin.sin_family = AF_INET;
    udp_addrin.sin_addr.s_addr = inet_addr(udp_addr);
    udp_addrin.sin_port = htons(udp_port);
    udp_fd = socket(AF_INET,SOCK_DGRAM,0);
}

int udp_send(u8* buf, int len)
{
    return sendto(udp_fd, buf, len, 0,
                  (sockaddr*)&udp_addrin,sizeof(udp_addrin));
}

int udp_recv(u8* buf, int len)
{
    unsigned int addrlen = sizeof(udp_addrin);
    return recvfrom(udp_fd, buf, len, 0, (sockaddr*)&udp_addrin, &addrlen);
}
