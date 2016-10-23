#include "network.h"
#include "error.h"

APSocket gSocket = -1;
APSocket gSocketBroad = -1;
APNetworkAddress gControllerSockaddr;

/**
 * inner function, get local ip address and default gateway
 * @param  sockFd [don't have to care about]
 * @param  bufPtr [don't have to care about]
 * @param  seqNum [don't have to care about]
 * @param  pId    [don't have to care about]
 * @return        [don't have to care about]
 */
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

/**
 * inner function, get local ip address and default gateway
 * @param nlHdr               [don't have to care about]
 * @param localIP             [don't have to care about]
 * @param localDefaultGateway [don't have to care about]
 */
void APNetworkParseRoutes(struct nlmsghdr *nlHdr, u32* localIP, u32* localDefaultGateway)
{
	struct route_info {
		struct in_addr dstAddr;
		struct in_addr srcAddr;
		struct in_addr gateWay;
		char ifName[IF_NAMESIZE];
	} rtInfo;
	struct route_info* prtInfo = &rtInfo;
	AP_ZERO_MEMORY(prtInfo, sizeof(struct route_info));

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
		*localDefaultGateway = ntohl((prtInfo->gateWay).s_addr);
		//sprintf(gAPDefaultGateway, (char *) inet_ntoa(prtInfo->gateWay));
	//sprintf(gLocalAddr, (char *) inet_ntoa(prtInfo->srcAddr));
	*localIP = ntohl((prtInfo->srcAddr).s_addr);

	return;
}

/**
 * init broadcast socket
 * @return [whether the operation is success or not]
 */
APBool APNetworkInitBroadcast()
{
	if(gSocketBroad >= 0) APNetworkCloseSocket(gSocketBroad);
	gSocketBroad = socket(AF_INET, SOCK_DGRAM, 0);
	if(gSocketBroad < 0) return AP_FALSE;
	const int opt = 1;
	int nb = setsockopt(gSocketBroad, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
	if(nb < 0) return AP_FALSE;
	return AP_TRUE;
}


/**
 * get local ip address, mac address and default gateway
 * @param  localIP             [output local ip address]
 * @param  localMAC            [output local mac address]
 * @param  localDefaultGateway [output local default gateway]
 * @return                     [whether the operation is success or not]
 */
APBool APNetworkInitLocalAddr(u32* localIP, u8* localMAC, u32* localDefaultGateway)
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
		APNetworkParseRoutes(nlMsg, localIP, localDefaultGateway);
	close(sock);
	
	//mac
	struct ifreq ifr;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, "eth0");
	if(!ioctl(sock, SIOCGIFHWADDR, &ifr)) {
		memcpy(localMAC, ifr.ifr_hwaddr.sa_data, 6);
	}
	else return AP_FALSE;
	return AP_TRUE;
}

/**
 * use controller's ip address to init sockaddr_in construction & singlecast socket
 * @param  controllerAddr [controller's ip address]
 * @return                [whether the operation is success or not]
 */
APBool APNetworkInitControllerAddr(u32 controllerAddr)
{
	if(gSocket >= 0) APNetworkCloseSocket(gSocket);
	gSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(gSocket < 0) return AP_FALSE;

	AP_ZERO_MEMORY(&gControllerSockaddr, sizeof(gControllerSockaddr));
	gControllerSockaddr.sin_family = AF_INET;
	gControllerSockaddr.sin_addr.s_addr = htonl(controllerAddr);
	gControllerSockaddr.sin_port = htons(PROTOCOL_PORT);
	
	return AP_TRUE;
}

/**
 * close a socket
 * @param s [socket you want to close]
 */
void APNetworkCloseSocket(APSocket s)
{
	shutdown(SHUT_RDWR, s);
	close(s);
	s = -1;
}

/**
 * send a singlecast msg to controller
 * @param  sendMsg [msg]
 * @return         [whether the operation is success or not]
 */
APBool APNetworkSend(APProtocolMessage sendMsg) 
{
	if(sendMsg.msg == NULL) 
		return AP_FALSE;
	while(sendto(gSocket, sendMsg.msg, sendMsg.offset, 0, (struct sockaddr*)&gControllerSockaddr, sizeof(gControllerSockaddr)) < 0) {
		if(errno == EINTR) continue;
		printf("%s\n", strerror(errno));
	}
	return AP_TRUE;
}

/**
 * send a broadcast msg to controller
 * @param  sendMsg [msg]
 * @return         [whether the operation is success or not]
 */
APBool APNetworkSendToBroad(APProtocolMessage sendMsg) 
{
	if(sendMsg.msg == NULL) 
		return AP_FALSE;
	APNetworkAddress broadAddr;
	AP_ZERO_MEMORY(&broadAddr, sizeof(broadAddr));
	broadAddr.sin_family = AF_INET;
	broadAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
	broadAddr.sin_port = htons(PROTOCOL_PORT);
	if(sendto(gSocketBroad, sendMsg.msg, sendMsg.offset, 0, (struct sockaddr*)&broadAddr, sizeof(gControllerSockaddr)) < 0) {
		return AP_FALSE;
	}
	return AP_TRUE;
}

/**
 * receive a singlecast msg from controller
 * @param  buffer     [a buffer to store content]
 * @param  bufferLen  [size of buffer]
 * @param  readLenPtr [output size of received msg]
 * @return            [whether the operation is success or not]
 */
APBool APNetworkReceive(u8* buffer,
					 int bufferLen, int* readLenPtr) 
{
	if(buffer == NULL || readLenPtr == NULL)
		return AP_FALSE;
	unsigned int  addrLen = sizeof(APNetworkAddress);
	*readLenPtr = recvfrom(gSocket, (char*)buffer, bufferLen, 0, (struct sockaddr*)&gControllerSockaddr, &addrLen);
	return AP_TRUE;
}

/**
 * receive a broadcast msg from controller
 * @param  buffer     [a buffer to store content]
 * @param  bufferLen  [size of buffer]
 * @param  readLenPtr [output size of received msg]
 * @return            [whether the operation is success or not]
 */
APBool APNetworkReceiveFromBroad(u8* buffer,
					 int bufferLen, int* readLenPtr) 
{
	if(buffer == NULL || readLenPtr == NULL)
		return AP_FALSE;
	unsigned int  addrLen = sizeof(APNetworkAddress);
	*readLenPtr = recvfrom(gSocketBroad, (char*)buffer, bufferLen, 0, (struct sockaddr*)&gControllerSockaddr, &addrLen);
	return AP_TRUE;
}

/**
 * wrapper for select to implement timer
 * @param  sock     [selected socket]
 * @param  timeout  [timeout]
 * @return            [whether the operation is success or not]
 */
APBool APNetworkTimedPollRead(APSocket sock, struct timeval *timeout) {
	int r;
	
	fd_set fset;
	
	if(timeout == NULL) return AP_FALSE;
	
	FD_ZERO(&fset);
	FD_SET(sock, &fset);

	if((r = select(sock+1, &fset, NULL, NULL, timeout)) == 0) 
	{
		return APErrorRaise(AP_ERROR_TIME_EXPIRED, NULL);
	} 
	else if (r < 0) 
	{
		if(errno == EINTR)
		{
			return APErrorRaise(AP_ERROR_INTERRUPTED, NULL);
		}
	}

	return AP_TRUE;
}
