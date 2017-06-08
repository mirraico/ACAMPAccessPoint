#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "common.h"
#include "error.h"
#include "protocol.h"

/*
for getting local network info
 */
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

extern int ap_socket;
extern int ap_socket_br;

bool init_local_addr(u32* localIP, u8* localMAC, u32* localDefaultGateway);
bool init_controller_addr(u32 controllerAddr);
bool init_broadcast();
void close_socket(int s);

bool send_udp(
					protocol_msg send_msg);
bool send_udp_br(
					protocol_msg send_msg);
bool recv_udp(u8* buffer,
					 int bufferLen, struct sockaddr_in* addr, int* readLenPtr);
bool recv_udp_br(u8* buffer,
					 int bufferLen, struct sockaddr_in* addr, int* readLenPtr);

bool time_poll_read(int sock, struct timeval *timeout);

#endif
