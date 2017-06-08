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

bool APNetworkInitLocalAddr(u32* localIP, u8* localMAC, u32* localDefaultGateway);
bool APNetworkInitControllerAddr(u32 controllerAddr);
bool APNetworkInitBroadcast();
void APNetworkCloseSocket(APSocket s);

bool APNetworkSend(
					APProtocolMessage sendMsg);
bool APNetworkSendToBroad(
					APProtocolMessage sendMsg);
bool APNetworkReceive(u8* buffer,
					 int bufferLen, APNetworkAddress* addr, int* readLenPtr);
bool APNetworkReceiveFromBroad(u8* buffer,
					 int bufferLen, APNetworkAddress* addr, int* readLenPtr);

#endif
