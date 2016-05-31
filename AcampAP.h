#ifndef __ACAMPAP_H__
#define __ACAMPAP_H__

#include "Common.h"

typedef struct
{
	u8 keyNum;
	u8 keyLength;
	void* key;
}WEPKey;

typedef struct
{
	u8 KeepAliveInterval;
	u8 DiscoverInterval;
	u8 StatisticTimer;
	u8 StateTimer;
	u8 IdleTimeout;
	u8 DiscoverTimeout;
	u8 RegisterTimeout;
	u8 ReportInterval;
}Timers;

extern u16 gAPID;
extern char gLocalAddr[20];
extern char gLocalDefaultGateway[20];

extern char gAPName[32];
extern char gAPDescriptor[128];
extern u8 gAPIPAddr[4];
extern u8 gAPMacAddr[6];

extern u8 gCntlName[32];
extern u8 gCntlDescriptor[128];
extern u8 gCntlIPAddr[4]; 
extern u8 gCntlMacAddr[6];

extern u8 gSSID[32];
extern u8 gChannel;
extern u8 gHwMode;
extern u8 gSuppressSSID;
extern u8 gSecuritySetting;
extern u8 gWPAVersion;
extern u8 gWPAPasswd[63];
extern u8 gWPAKeyManagement;
extern u8 gWPAPairwise;
extern u8 gWPARekey[4];
extern u8 gDefaultWEPKey;
extern WEPKey gWEPKeys[4];
extern u8 gMACACLMod;
extern u8* gMACAcceptList;
extern u8* gMACDenyList;

extern u16 gCountryCode;
extern u8 gEnableDot11d;
extern u8 gAddSTAMAC[6];
extern u8 gDelSTAMAC[6];
extern u32 gStateDescriptor;

__inline__ u16 APGetAPID() { return gAPID; }
__inline__ char* APGetLocalAddr() { return gLocalAddr; }
__inline__ char* APGetLocalDefaultGateway() { return gLocalDefaultGateway; }
__inline__ char* APGetAPName() { return gAPName; }

#endif // ACAMPAP_H

