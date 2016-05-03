#include "AcampNetwork.h"

char gAddress[20];
int gPort;
APSocket gSocket;
APNetworkAddress gSockaddr;

int APNetworkGetAddressSize()
{
	return sizeof(struct sockaddr_in);
}

APBool APNetworkInitLocalAddr()
{
	int sfd, intr;
	struct ifreq buf[16];
	struct ifconf ifc;
	sfd = socket (AF_INET, SOCK_DGRAM, 0);
	if (sfd < 0) return AP_FALSE;
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t)buf;
	if (ioctl(sfd, SIOCGIFCONF, (char *)&ifc)) return AP_FALSE;
	intr = ifc.ifc_len / sizeof(struct ifreq);
	while (intr-- > 0 && ioctl(sfd, SIOCGIFADDR, (char *)&buf[intr]));
	close(sfd);
	char* localaddr = inet_ntoa(((struct sockaddr_in*)(&buf[intr].ifr_addr))->sin_addr);
	int localaddrlen = strlen(localaddr);
	AP_ZERO_MEMORY(gLocalAddr, 20);
	AP_COPY_MEMORY(gLocalAddr, localaddr, localaddrlen + 1);
	return AP_TRUE;
}

APBool APNetworkInit()
{
	gSockaddr.sin_family = AF_INET;
	gSockaddr.sin_addr.s_addr = inet_addr(gAddress);
	gSockaddr.sin_port = htons(gPort);
	gSocket = socket(AF_INET,SOCK_DGRAM,0);
	
	if(!APNetworkInitLocalAddr()) return AP_FALSE;
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
