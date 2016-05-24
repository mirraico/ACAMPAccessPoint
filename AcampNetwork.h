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
extern APNetworkAddress gServerSockaddr;
extern char gControllerAddr[20];
extern int gPort;

APBool APNetworkInit();
APBool APNetworkInitBroad();
int APNetworkGetAddressSize();
APBool APNetworkInitLocalAddr();
APBool APNetworkSendUnconnected(APSocket sock, 
					APNetworkAddress *addrPtr,
					const u8 *buf,
					int len);
APBool APNetworkSendToServerUnconnected(
					const u8 *buf,
					int len);
APBool APNetworkSendToBroadUnconnected(
					const u8 *buf,
					int len);

#define		APNetworkCloseSocket(x)		{ shutdown(SHUT_RDWR, x); close(x); }

#endif
