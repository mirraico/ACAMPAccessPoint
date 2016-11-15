#include "ap.h"

u32 gSeqNum;
u32 gWaitSeqNum;

u16 gAPID;
u8 gDiscoveryType;
u8 gRegisteredService;

u32 gStaticControllerIPAddr; 

char* gControllerName;
char* gControllerDescriptor;
u32 gControllerIPAddr; 
u8 gControllerMACAddr[6];

char* gAPName;
char* gAPDescriptor;
u32 gAPIPAddr;
u8 gAPMACAddr[6];
u32 gAPDefaultGateway;

int gHdSysLogModules;
int gHdSysLogLevel;
int gHdStdoutModules;
int gHdStdoutLevel;

char* gSSID;
u8 gChannel;
u8 gHardwareMode;
u8 gSuppressSSID;
u8 gSecurityOption;

APWEP gWEP;
APWPA gWPA;

__inline__ u32 APGetSeqNum() { return gSeqNum; }
__inline__ u32 APGetWaitSeqNum() { return gWaitSeqNum; }

__inline__ u16 APGetAPID() { return gAPID; }
void APSetAPID(u16 apid) { gAPID = apid; }
__inline__ u8 APGetDiscoveryType() { return gDiscoveryType; }
__inline__ u8 APGetRegisteredService() { return gRegisteredService; }

__inline__ u32 APGetStaticControllerIPAddr() { return gStaticControllerIPAddr; }

__inline__ char* APGetControllerName() { return gControllerName; }
__inline__ char* APGetControllerDescriptor() { return gControllerDescriptor; }
__inline__ u32 APGetControllerIPAddr() { return gControllerIPAddr; }
__inline__ u8* APGetControllerMACAddr() { return gControllerMACAddr; }

__inline__ char* APGetAPName() { return gAPName; }
__inline__ char* APGetAPDescriptor() { return gAPDescriptor; }
__inline__ u32 APGetAPIPAddr() { return gAPIPAddr; }
__inline__ u8* APGetAPMACAddr() { return gAPMACAddr; }
__inline__ u32 APGetAPDefaultGateway() { return gAPDefaultGateway; }

__inline__ int APGetHdSysLogModules() { return gHdSysLogModules; }
__inline__ int APGetHdSysLogLevel() { return gHdSysLogLevel; }
__inline__ int APGetHdStdoutModules() { return gHdStdoutModules; }
__inline__ int APGetHdStdoutLevel() { return gHdStdoutLevel; }

__inline__ char* APGetSSID() { return gSSID; }
__inline__ u8 APGetChannel() { return gChannel; }
__inline__ u8 APGetHardwareMode() { return gHardwareMode; }
__inline__ u8 APGetSuppressSSID() { return gSuppressSSID; }
__inline__ u8 APGetSecurityOption() { return gSecurityOption; }

__inline__ APWEP* APGetWEP() { return &gWEP; }
__inline__ APWPA* APGetWPA() { return &gWPA; }


/**
 * init ap, including seqnum, apid, etc, and set random seed
 */
void APInit()
{
	srand( (unsigned)time( NULL ) );

	gSeqNum = rand();
	gWaitSeqNum = gSeqNum;
	gAPID = 0;
}

