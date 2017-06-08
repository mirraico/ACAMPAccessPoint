#include "network.h"

int ap_socket = -1;
int ap_socket_br = -1;
static struct sockaddr_in controller_sockaddr;

/**
 * inner function, get local ip address and default gateway
 * @param  sock_fd [don't have to care about]
 * @param  buf_ptr [don't have to care about]
 * @param  seq_num [don't have to care about]
 * @param  pid    [don't have to care about]
 * @return        [don't have to care about]
 */
static int read_nl_sock(int sock_fd, char *buf_ptr, unsigned int seq_num, unsigned int pid)
{
	struct nlmsghdr *nlhdr;
	int readLen = 0, msgLen = 0;
	do 
	{
		if ((readLen = recv(sock_fd, buf_ptr, 8192 - msgLen, 0)) < 0) return -1;
		nlhdr = (struct nlmsghdr *) buf_ptr;

		if ((NLMSG_OK(nlhdr, readLen) == 0)
			|| (nlhdr->nlmsg_type == NLMSG_ERROR))
			return -1;

		if (nlhdr->nlmsg_type == NLMSG_DONE) {
			break;
		} else {
			buf_ptr += readLen;
			msgLen += readLen;
		}

		if ((nlhdr->nlmsg_flags & NLM_F_MULTI) == 0) break;
	} while ((nlhdr->nlmsg_seq != seq_num) || (nlhdr->nlmsg_pid != pid));
	return msgLen;
}

/**
 * inner function, get local ip address and default gateway
 * @param nlhdr               [don't have to care about]
 * @param local_ip             [don't have to care about]
 * @param local_de_gw [don't have to care about]
 */
static void parse_routes(struct nlmsghdr *nlhdr, u32* local_ip, u32* local_de_gw)
{
	struct route_info {
		struct in_addr dstAddr;
		struct in_addr srcAddr;
		struct in_addr gateWay;
		char ifName[IF_NAMESIZE];
	} rtInfo;
	struct route_info* prtInfo = &rtInfo;
	zero_memory(prtInfo, sizeof(struct route_info));

	struct rtmsg *rtMsg = (struct rtmsg *) NLMSG_DATA(nlhdr);
	if ((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN)) return;
	struct rtattr *rtAttr = (struct rtattr *) RTM_RTA(rtMsg);
	int rtLen = RTM_PAYLOAD(nlhdr);
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

	/* use wired ethernet to connect with controller */
	// if(strncmp(prtInfo->ifName, "en", 2) != 0 && strncmp(prtInfo->ifName, "eth", 3) != 0) return;
	// if(strncmp(prtInfo->ifName, "wl", 2) != 0) return;
	if(strcmp(prtInfo->ifName, ap_ethname) != 0) return;

	if (prtInfo->dstAddr.s_addr == 0)
		*local_de_gw = ntohl((prtInfo->gateWay).s_addr);
		//sprintf(ap_default_gw, (char *) inet_ntoa(prtInfo->gateWay));
	//sprintf(gLocalAddr, (char *) inet_ntoa(prtInfo->srcAddr));
	*local_ip = ntohl((prtInfo->srcAddr).s_addr);

	return;
}

/**
 * init broadcast socket
 * @return [whether the operation is success or not]
 */
bool init_broadcast()
{
	if(ap_socket_br >= 0) close_socket(ap_socket_br);
	ap_socket_br = socket(AF_INET, SOCK_DGRAM, 0);
	if(ap_socket_br < 0) return false;
	const int opt = 1;
	int nb = setsockopt(ap_socket_br, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
	if(nb < 0) return false;
	return true;
}


/**
 * get local ip address, mac address and default gateway
 * @param  local_ip             [output local ip address]
 * @param  localMAC            [output local mac address]
 * @param  local_de_gw [output local default gateway]
 * @return                     [whether the operation is success or not]
 */
bool init_local_addr(u32* local_ip, u8* localMAC, u32* local_de_gw)
{
	char msgBuf[8192];
	int sock, len, msgSeq = 0;

	if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0) return false;
	zero_memory(msgBuf, 8192);

	struct nlmsghdr *nlMsg = (struct nlmsghdr *) msgBuf;
	nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	nlMsg->nlmsg_type = RTM_GETROUTE;
	nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
	nlMsg->nlmsg_seq = msgSeq++;
	nlMsg->nlmsg_pid = getpid();

	if (send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0)  return false;
	if ((len = read_nl_sock(sock, msgBuf, msgSeq, getpid())) < 0) return false;
	
	for (; NLMSG_OK(nlMsg, len); nlMsg = NLMSG_NEXT(nlMsg, len)) 
		parse_routes(nlMsg, local_ip, local_de_gw);
	close(sock);
	
	/* mac addr */
	struct ifreq ifr;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	/* use wired ethernet to connect with controller */
	strcpy(ifr.ifr_name, ap_ethname);
	if(!ioctl(sock, SIOCGIFHWADDR, &ifr)) {
		memcpy(localMAC, ifr.ifr_hwaddr.sa_data, 6);
	}
	else return false;
	return true;
}

/**
 * use controller's ip address to init sockaddr_in construction & singlecast socket
 * @param  controllerAddr [controller's ip address]
 * @return                [whether the operation is success or not]
 */
bool init_controller_addr(u32 controllerAddr)
{
	if(ap_socket >= 0) close_socket(ap_socket);
	ap_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(ap_socket < 0) return false;

	zero_memory(&controller_sockaddr, sizeof(controller_sockaddr));
	controller_sockaddr.sin_family = AF_INET;
	controller_sockaddr.sin_addr.s_addr = htonl(controllerAddr);
	controller_sockaddr.sin_port = htons(PROTOCOL_PORT);
	
	return true;
}

/**
 * close a socket
 * @param s [socket you want to close]
 */
void close_socket(int s)
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
bool send_udp(APProtocolMessage sendMsg) 
{
	if(sendMsg.msg == NULL) 
		return false;
	if(sendto(ap_socket, sendMsg.msg, sendMsg.offset, 0, (struct sockaddr*)&controller_sockaddr, sizeof(controller_sockaddr)) < 0) {
		return false;
	}
	return true;
}

/**
 * send a broadcast msg to controller
 * @param  sendMsg [msg]
 * @return         [whether the operation is success or not]
 */
bool send_udp_br(APProtocolMessage sendMsg) 
{
	if(sendMsg.msg == NULL) 
		return false;
	struct sockaddr_in broadAddr;
	zero_memory(&broadAddr, sizeof(broadAddr));
	broadAddr.sin_family = AF_INET;
	broadAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
	broadAddr.sin_port = htons(PROTOCOL_PORT);
	if(sendto(ap_socket_br, sendMsg.msg, sendMsg.offset, 0, (struct sockaddr*)&broadAddr, sizeof(controller_sockaddr)) < 0) {
		return false;
	}
	return true;
}

/**
 * receive a singlecast msg from controller
 * @param  buffer     [a buffer to store content]
 * @param  bufferLen  [size of buffer]
 * @param  addr       [addr of controller]
 * @param  readLenPtr [output size of received msg]
 * @return            [whether the operation is success or not]
 */
bool recv_udp(u8* buffer,
					 int bufferLen, struct sockaddr_in* addr, int* readLenPtr) 
{
	if(buffer == NULL || readLenPtr == NULL)
		return false;
	unsigned int  addrLen = sizeof(struct sockaddr_in);
	if((*readLenPtr = recvfrom(ap_socket, (char*)buffer, bufferLen, 0, (struct sockaddr*)addr, &addrLen)) < 0) {
		return false;
	}
	return true;
}

/**
 * receive a broadcast msg from controller
 * @param  buffer     [a buffer to store content]
 * @param  bufferLen  [size of buffer]
 * @param  addr       [addr of controller]
 * @param  readLenPtr [output size of received msg]
 * @return            [whether the operation is success or not]
 */
bool recv_udp_br(u8* buffer,
					 int bufferLen, struct sockaddr_in* addr, int* readLenPtr) 
{
	if(buffer == NULL || readLenPtr == NULL)
		return false;
	unsigned int addrLen = sizeof(struct sockaddr_in);
	if((*readLenPtr = recvfrom(ap_socket_br, (char*)buffer, bufferLen, 0, (struct sockaddr*)addr, &addrLen)) < 0) {
		return false;
	}
	return true;
}

/**
 * wrapper for select to implement timer
 * @param  sock     [selected socket]
 * @param  timeout  [timeout]
 * @return            [whether the operation is success or not]
 */
bool time_poll_read(int sock, struct timeval *timeout) {
	int r;
	
	fd_set fset;
	
	if(timeout == NULL) return false;
	
	FD_ZERO(&fset);
	FD_SET(sock, &fset);

	if((r = select(sock+1, &fset, NULL, NULL, timeout)) == 0) 
	{
		return false;
	} 
	else if (r < 0) 
	{
		log_e("Select Error");
		if(errno == EINTR)
		{
			log_e("Select Interrupted by signal");
			return false;
		}
		return false;
	}

	return true;
}
