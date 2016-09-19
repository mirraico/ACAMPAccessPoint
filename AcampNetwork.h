#ifndef __ACAMPNETWORK_H__
#define __ACAMPNETWORK_H__

#include "Common.h"
#include "AcampProtocol.h"

/*
for getting local network info
 */
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

typedef int APSocket;
typedef struct sockaddr_in APNetworkAddress;

extern APSocket gSocket;
extern APSocket gSocketBroad;
extern APNetworkAddress gControllerSockaddr;

APBool APNetworkInit();
APBool APNetworkInitLocalAddr();
APBool APNetworkInitControllerAddr();
void APNetworkCloseSocket(APSocket s);

APBool APNetworkSend(
					APProtocolMessage sendMsg) ;
APBool APNetworkSendToBroad(
					APProtocolMessage sendMsg);
APBool APNetworkReceive(u8* buffer,
					 int bufferLen, int* readLenPtr);
APBool APNetworkReceiveFromBroad(u8* buffer,
					 int bufferLen, int* readLenPtr);

#endif
