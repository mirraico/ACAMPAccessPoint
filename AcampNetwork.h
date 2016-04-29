#ifndef __ACAMPNETWORK_H__
#define __ACAMPNETWORK_H__

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netdb.h>
#include "Common.h"

char gAddress[20];
int gPort;
typedef int APSocket;
typedef struct sockaddr_in APNetworkAddress;

APSocket gSocket;
APNetworkAddress gSockaddr;

APBool APNetworkInit();
int APNetworkGetAddressSize();
APBool APNetworkSendMessage(const u8 *buf, int len);


#define		APNetworkCloseSocket(x)		{ shutdown(SHUT_RDWR, x); close(x); }

#endif
