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

typedef int APSocket;
typedef struct sockaddr_in APNetworkAddress;

extern APSocket gSocket;
extern APSocket gSocketBroad;

APBool APNetworkInitLocalAddr(u32* localIP, u8* localMAC, u32* localDefaultGateway);
APBool APNetworkInitControllerAddr(u32 controllerAddr);
APBool APNetworkInitBroadcast();
void APNetworkCloseSocket(APSocket s);

APBool APNetworkSend(
					APProtocolMessage sendMsg);
APBool APNetworkSendToBroad(
					APProtocolMessage sendMsg);
APBool APNetworkReceive(u8* buffer,
					 int bufferLen, APNetworkAddress* addr, int* readLenPtr);
APBool APNetworkReceiveFromBroad(u8* buffer,
					 int bufferLen, APNetworkAddress* addr, int* readLenPtr);

#endif
