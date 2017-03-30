#include "ap.h"

u32 gSeqNum;
u32 gControllerSeqNum;

int gRetransmitInterval;
int gKeepAliveInterval;
int gMaxRetransmit;

u16 gAPID;
u8 gDiscoveryType;
u8 gRegisteredService;

char* gIfEthName;
u32 gStaticControllerIPAddr; 
u32 gControllerIPAddr; 
u8 gControllerMACAddr[6];
u32 gAPIPAddr;
u8 gAPMACAddr[6];
u32 gAPDefaultGateway; //unused

char* gControllerName;
char* gControllerDescriptor;
char* gAPName;
char* gAPDescriptor;

/* all wireless conf */
struct wlconf *wlconf;
APBool wlflag;

__inline__ u32 APGetSeqNum() { return gSeqNum; }
__inline__ u32 APGetControllerSeqNum() { return gControllerSeqNum; }
void APSetControllerSeqNum(u32 controllerSeqNum) { gControllerSeqNum = controllerSeqNum; }

__inline__ u16 APGetAPID() { return gAPID; }
void APSetAPID(u16 apid) { gAPID = apid; }

/**
 * init ap, including seqnum, apid, etc, and set random seed
 */
void APInitProtocol()
{
	srand( (unsigned)time( NULL ) );

	gSeqNum = rand();
	gAPID = 0;

	gRetransmitInterval = 3;
	gMaxRetransmit = 5;
	gKeepAliveInterval = 30;
}

