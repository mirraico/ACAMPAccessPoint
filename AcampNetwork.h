#ifndef __ACAMPNETWORK_H__
#define __ACAMPNETWORK_H__

#include "Common.h"
#include "AcampAP.h"

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

extern char gAddress[20];
extern int gPort;

typedef int APSocket;
typedef struct sockaddr_in APNetworkAddress;


APBool APNetworkInit();
int APNetworkGetAddressSize();
APBool APNetworkInitLocalAddr();
APBool APNetworkSendMessage(const u8 *buf, int len);


#define		APNetworkCloseSocket(x)		{ shutdown(SHUT_RDWR, x); close(x); }

#endif
