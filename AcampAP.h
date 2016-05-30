#ifndef __ACAMPAP_H__
#define __ACAMPAP_H__

#include "Common.h"

extern u16 gAPID;
extern char gAPName[32];
extern char gAPBoardData[64];
extern char gAPDescriptor[128];
extern char gLocalAddr[20];
extern char gLocalDefaultGateway[20];


extern u16 ResultCode;
extern u16 ReasonCode;
extern u16 AssignedAPID;
extern u8 CntlName[32];
extern u8 CntlDescriptor[128];
extern u8 CntlIPAddr[4]; 
extern u8 CntlMacAddr[6];
extern u8 APName[32];
extern u8 APDescriptor[128];
extern u8 APIPAddr[4];
extern u8 APMacAddr[6];
extern u8 SSID[32];
extern u8 Channel;
extern u8 HwMode;
extern u8 SuppressSSID;
extern u8 SecuritySetting;
extern u8 WPAVersion;
extern u8 WPAPasswd[63];
extern u8 WPAKeyMange;
extern u8 WPAPairwise;
extern u8 gWPARekey[4];
extern u8 DefaultWEPKey;
typedef struct
{
	u8 NumKey;
	u8 KeyLength;
	void* WEPKey;
}WEPKey;
extern u8 MACACLMod;
typedef struct
{
	u8 NumMac;
	u8 * MACAddr[6];
}lMACAccept;
typedef struct
{
	u8 NumMac;
	u8 *MACAddr[6];
}lMACDeny;
extern u16 CountryCode;
extern u8 EnableDot11d;
extern u8 AddSTA[6];
extern u8 DelSTA[6];
extern u32 StateDescriptor;
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

}TimersStatistics;


__inline__ u16 APGetAPID() { return gAPID; }
__inline__ char* APGetAPName() { return gAPName; }
__inline__ char* APGetAPBoardData() { return gAPBoardData; }
__inline__ char* APGetAPDescriptor() { return gAPDescriptor; }
__inline__ char* APGetLocalAddr() { return gLocalAddr; }
__inline__ char* APGetLocalDefaultGateway() { return gLocalDefaultGateway; }

#endif // ACAMPAP_H

