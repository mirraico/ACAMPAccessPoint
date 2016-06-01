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

extern char gAPName[32];
extern char gAPDescriptor[128];
extern u32 gAPIPAddr;
extern u8 gAPMACAddr[6];
extern u32 gAPDefaultGateway;

extern char* gControllerName;
extern char* gControllerDescriptor;
extern u32 gControllerIPAddr; 
extern u8 gControllerMacAddr[6];

extern char* gSSID;
extern u8 gChannel;
extern u8 gHwMode;
extern u8 gSuppressSSID;
extern u8 gSecuritySetting;
extern u8 gWPAVersion;
extern char* gWPAPasswd;
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
__inline__ char* APGetAPName() { return gAPName; }
__inline__ char* APGetAPDescriptor() { return gAPDescriptor; }
__inline__ u32 APGetAPIPAddr() { return gAPIPAddr; }
__inline__ u8* APGetAPMACAddr() { return gAPMACAddr; }


__inline__ char* APGetSSID() { return gSSID; }
__inline__ u8 APGetChannel() { return gChannel; }
__inline__ u8 APGetHwMode() { return gHwMode; }
__inline__ u8 APGetSuppressSSID() { return gSuppressSSID; }
__inline__ u8 APGetSecuritySetting() { return gSecuritySetting; }
__inline__ u8 APGetWPAVersion() { return gWPAVersion; }
__inline__ char* APGetWPAPasswd() { return gWPAPasswd; }
__inline__ u8 APGetWPAKeyManagement() { return gWPAKeyManagement; }
__inline__ u8 APGetWPAPairwise() { return gWPAPairwise; }

#endif // ACAMPAP_H

