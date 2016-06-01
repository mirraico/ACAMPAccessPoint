#include "AcampNetwork.h"

u32 gNetworkControllerAddr;
int gNetworkPort = 6606;
APNetworkAddress gNetworkControllerSockaddr;
APSocket gSocket = -1;
APSocket gSocketBroad = -1;

int APNetworkGetAddressSize()
{
	return sizeof(struct sockaddr);
}

int APNetworkReadNlSock(int sockFd, char *bufPtr, unsigned int seqNum, unsigned int pId)
{
	struct nlmsghdr *nlHdr;
	int readLen = 0, msgLen = 0;
	do 
	{
		if ((readLen = recv(sockFd, bufPtr, 8192 - msgLen, 0)) < 0) return -1;
		nlHdr = (struct nlmsghdr *) bufPtr;

		if ((NLMSG_OK(nlHdr, readLen) == 0)
			|| (nlHdr->nlmsg_type == NLMSG_ERROR))
			return -1;

		if (nlHdr->nlmsg_type == NLMSG_DONE) {
			break;
		} else {
			bufPtr += readLen;
			msgLen += readLen;
		}

		if ((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0) break;
	} while ((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));
	return msgLen;
}

void APNetworkParseRoutes(struct nlmsghdr *nlHdr)
{
	struct route_info {
		struct in_addr dstAddr;
		struct in_addr srcAddr;
		struct in_addr gateWay;
		char ifName[IF_NAMESIZE];
	} rtInfo;
	struct route_info* prtInfo = &rtInfo;
	AP_ZERO_MEMORY(prtInfo, sizeof(route_info));

	struct rtmsg *rtMsg = (struct rtmsg *) NLMSG_DATA(nlHdr);
	if ((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN)) return;
	struct rtattr *rtAttr = (struct rtattr *) RTM_RTA(rtMsg);
	int rtLen = RTM_PAYLOAD(nlHdr);
	for (; RTA_OK(rtAttr, rtLen); rtAttr = RTA_NEXT(rtAttr, rtLen)) {
		switch (rtAttr->rta_type) {
		case RTA_OIF:
			if_indextoname(*(int *) RTA_DATA(rtAttr), prtInfo->ifName);
			break;
		case RTA_GATEWAY:
			prtInfo->gateWay.s_addr= *(u_int *) RTA_DATA(rtAttr);
			break;
		case RTA_PREFSRC:
			prtInfo->srcAddr.s_addr= *(u_int *) RTA_DATA(rtAttr);
			break;
		case RTA_DST:
			prtInfo->dstAddr .s_addr= *(u_int *) RTA_DATA(rtAttr);
			break;
		}
	}

	if (prtInfo->dstAddr.s_addr == 0)
		gAPDefaultGateway = ntohl((prtInfo->gateWay).s_addr);
		//sprintf(gAPDefaultGateway, (char *) inet_ntoa(prtInfo->gateWay));
	//sprintf(gLocalAddr, (char *) inet_ntoa(prtInfo->srcAddr));
	gAPIPAddr = ntohl((prtInfo->srcAddr).s_addr);

	return;
}

APBool APNetworkInitLocalAddr()
{
	char msgBuf[8192];
	int sock, len, msgSeq = 0;

	if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0) return AP_FALSE;
	AP_ZERO_MEMORY(msgBuf, 8192);

	struct nlmsghdr *nlMsg = (struct nlmsghdr *) msgBuf;
	nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	nlMsg->nlmsg_type = RTM_GETROUTE;
	nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
	nlMsg->nlmsg_seq = msgSeq++;
	nlMsg->nlmsg_pid = getpid();

	if (send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0)  return AP_FALSE;
	if ((len = APNetworkReadNlSock(sock, msgBuf, msgSeq, getpid())) < 0) return AP_FALSE;
	
	for (; NLMSG_OK(nlMsg, len); nlMsg = NLMSG_NEXT(nlMsg, len)) 
		APNetworkParseRoutes(nlMsg);
	close(sock);
	
	//mac
	struct ifreq ifr;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, "eth0");
	if(!ioctl(sock, SIOCGIFHWADDR, &ifr)) {
		memcpy(gAPMACAddr, ifr.ifr_hwaddr.sa_data, 6);
	}
	else return AP_FALSE;
	return AP_TRUE;
}

APBool APNetworkInit()
{
	if(gSocket >= 0) APNetworkCloseSocket(gSocket);
	gSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(gSocket < 0) return AP_FALSE;
	if(gSocketBroad >= 0) APNetworkCloseSocket(gSocketBroad);
	gSocketBroad = socket(AF_INET, SOCK_DGRAM, 0);
	if(gSocketBroad < 0) return AP_FALSE;
	const int opt = 1;
	int nb = setsockopt(gSocketBroad, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
	if(nb < 0) return AP_FALSE;
	return AP_TRUE;
}

APBool APNetworkInitControllerAddr()
{
	AP_ZERO_MEMORY(&gNetworkControllerSockaddr, sizeof(gNetworkControllerSockaddr));
	gNetworkControllerSockaddr.sin_family = AF_INET;
	gNetworkControllerSockaddr.sin_addr.s_addr = htonl(gNetworkControllerAddr);
	gNetworkControllerSockaddr.sin_port = htons(gNetworkPort);
	
	return AP_TRUE;
}

APBool APNetworkInitControllerAddr(APNetworkAddress addr)
{
	AP_ZERO_MEMORY(&gNetworkControllerSockaddr, sizeof(gNetworkControllerSockaddr));
	gNetworkControllerSockaddr = addr;
	
	return AP_TRUE;
}

APBool APNetworkSendUnconnected(APProtocolMessage sendMsg) {
	if(sendMsg.msg == NULL) 
		return AP_FALSE;
	while(sendto(gSocket, sendMsg.msg, sendMsg.offset, 0, (struct sockaddr*)&gNetworkControllerSockaddr, APNetworkGetAddressSize()) < 0) {
		if(errno == EINTR) continue;
		printf("%s\n", strerror(errno));
	}
	return AP_TRUE;
}

APBool APNetworkSendToBroadUnconnected(APProtocolMessage sendMsg) {
	if(sendMsg.msg == NULL) 
		return AP_FALSE;
	APNetworkAddress broadAddr;
	AP_ZERO_MEMORY(&broadAddr, sizeof(broadAddr));
	broadAddr.sin_family = AF_INET;
    broadAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
	broadAddr.sin_port = htons(gNetworkPort);
	if(sendto(gSocketBroad, sendMsg.msg, sendMsg.offset, 0, (struct sockaddr*)&broadAddr, APNetworkGetAddressSize()) < 0) {
		return AP_FALSE;
	}
	return AP_TRUE;
}

APBool APNetworkReceiveUnconnected(u8* buffer,
					 int bufferLen, int* readLenPtr, APNetworkAddress* addrPtr) {
    if(buffer == NULL || readLenPtr == NULL)
        return AP_FALSE;
    unsigned int  addrLen = sizeof(APNetworkAddress);
    *readLenPtr = recvfrom(gSocket, (char*)buffer, bufferLen, 0, (struct sockaddr*)addrPtr, &addrLen);
	return AP_TRUE;
}

APBool APNetworkReceiveFromBroadUnconnected(u8* buffer,
                     int bufferLen, int* readLenPtr, APNetworkAddress* addrPtr) {
    if(buffer == NULL || readLenPtr == NULL)
        return AP_FALSE;
    unsigned int  addrLen = sizeof(APNetworkAddress);
    *readLenPtr = recvfrom(gSocketBroad, (char*)buffer, bufferLen, 0, (struct sockaddr*)addrPtr, &addrLen);
    return AP_TRUE;
}
