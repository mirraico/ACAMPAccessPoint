#ifndef __ACAMPNETWORK_H__
#define __ACAMPNETWORK_H__

#include "Common.h"
#include "AcampAP.h"
#include "AcampProtocol.h"

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

typedef int APSocket;
typedef struct sockaddr_in APNetworkAddress;

extern APSocket gSocket;
extern APSocket gSocketBroad;
extern APNetworkAddress gNetworkControllerSockaddr;
extern u32 gNetworkControllerAddr;
extern int gNetworkPort;

APBool APNetworkInit();
APBool APNetworkInitLocalAddr();
APBool APNetworkInitControllerAddr();
APBool APNetworkInitControllerAddr(APNetworkAddress addr);
int APNetworkGetAddressSize();
APBool APNetworkSendUnconnected(
					APProtocolMessage sendMsg) ;
APBool APNetworkSendToBroadUnconnected(
					APProtocolMessage sendMsg);
APBool APNetworkReceiveUnconnected(u8* buffer,
					 int bufferLen, int* readLenPtr, APNetworkAddress* addrPtr);
APBool APNetworkReceiveFromBroadUnconnected(u8* buffer,
					 int bufferLen, int* readLenPtr, APNetworkAddress* addrPtr);

#define		APNetworkCloseSocket(x)		{ shutdown(SHUT_RDWR, x); close(x); x=-1;}

#endif
