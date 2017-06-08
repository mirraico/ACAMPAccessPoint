#ifndef __AP_H__
#define __AP_H__

#include "common.h"

extern char* gIfEthName;

extern u32 gSeqNum;
extern u32 gControllerSeqNum;

extern int gRetransmitInterval;
extern int gKeepAliveInterval;
extern int gMaxRetransmit;

extern u16 gAPID;
extern u8 gDiscoveryType;
extern u8 gRegisteredService;

extern u32 gStaticControllerIPAddr; 

extern char* gControllerName;
extern char* gControllerDescriptor;
extern u32 gControllerIPAddr; 
extern u8 gControllerMACAddr[6];

extern char* gAPName;
extern char* gAPDescriptor;
extern u32 gAPIPAddr;
extern u8 gAPMACAddr[6];
extern u32 gAPDefaultGateway;

extern struct wlconf *wlconf;
extern bool wlflag;

#define APSeqNumIncrement()         gSeqNum++
#define APControllerSeqNumIncrement()         gControllerSeqNum++

extern u32 APGetSeqNum();
extern u32 APGetControllerSeqNum();
void APSetControllerSeqNum(u32 controllerSeqNum);

extern u16 APGetAPID();
extern void APSetAPID(u16 apid);

void APInitProtocol();


#endif // AP_H