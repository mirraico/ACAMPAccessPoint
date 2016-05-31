#ifndef __ACAMPNETWORK_H__
#define __ACAMPNETWORK_H__

#include "Common.h"
#include "AcampAP.h"

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

typedef int APSocket;
typedef struct sockaddr_in APNetworkAddress;

extern APSocket gSocket;
extern APSocket gSocketBroad;
extern APNetworkAddress gControllerSockaddr;
extern char gControllerAddr[20];
extern int gPort;

APBool APNetworkInit();
APBool APNetworkInitControllerAddr();
APBool APNetworkInitControllerAddr(APNetworkAddress addr);
int APNetworkGetAddressSize();
APBool APNetworkSendUnconnected(
					const u8 *buf,
					int len) ;
APBool APNetworkSendToBroadUnconnected(
					const u8 *buf,
					int len);

#define		APNetworkCloseSocket(x)		{ shutdown(SHUT_RDWR, x); close(x); x=-1;}

#endif
