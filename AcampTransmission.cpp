#include "AcampTransmission.h"

sockaddr_in udp_addrin;
int udp_init(char* udp_addr, int udp_port)
{
    int udp_sock;
    udp_addrin.sin_family = AF_INET;
    udp_addrin.sin_addr.s_addr = inet_addr(udp_addr);
    udp_addrin.sin_port = htons(udp_port);
    udp_sock = socket(AF_INET,SOCK_DGRAM,0);
    return udp_sock;
}


int udp_send(int fd, u8* buf, int len)
{
    return sendto(fd, buf, len, 0,
                  (sockaddr*)&udp_addrin,sizeof(udp_addrin));
}
