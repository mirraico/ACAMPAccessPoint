#include "AcampNetwork.h"

int APNetworkGetAddressSize()
{
    return sizeof(struct sockaddr_in);
}

APBool APNetworkInit()
{
	gSockaddr.sin_family = AF_INET;
    gSockaddr.sin_addr.s_addr = inet_addr(gAddress);
    gSockaddr.sin_port = htons(gPort);
    gSocket = socket(AF_INET,SOCK_DGRAM,0);
	return AP_TRUE;
}

APBool APNetworkSendMessage(const u8 *buf, int len)
{
    if(buf == NULL)
        return AP_FALSE;

    if(sendto(gSocket, buf, len, 0,
                  (sockaddr*)&gSockaddr, APNetworkGetAddressSize()) < 0)
        return AP_FALSE;

    return AP_TRUE;
}
