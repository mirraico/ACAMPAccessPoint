#ifndef __AP_H__
#define __AP_H__

#include "common.h"

extern u32 gSeqNum;

extern u16 gAPID;
extern u8 gDiscoveryType;
extern u8 gRegisteredService;

extern char* gControllerName;
extern char* gControllerDescriptor;
extern u32 gControllerIPAddr; 
extern u8 gControllerMACAddr[6];

extern char* gAPName;
extern char* gAPDescriptor;
extern u32 gAPIPAddr;
extern u8 gAPMACAddr[6];
extern u32 gAPDefaultGateway;

extern char* gSSID;
extern u8 gChannel;
extern u8 gHardwareMode;
extern u8 gSuppressSSID;
extern u8 gSecuritySetting;


extern u32 APGetSeqNum();

extern u16 APGetAPID();
extern u8 APGetDiscoveryType();
extern u8 APGetRegisteredService();

extern char* APGetControllerName();
extern char* APGetControllerDescriptor();
extern u32 APGetControllerIPAddr(); 
extern u8* APGetControllerMACAddr();

extern char* APGetAPName();
extern char* APGetAPDescriptor();
extern u32 APGetAPIPAddr();
extern u8* APGetAPMACAddr();
extern u32 APGetAPDefaultGateway();

extern char* APGetSSID();
extern u8 APGetChannel();
extern u8 APGetHardwareMode();
extern u8 APGetSuppressSSID();
extern u8 APGetSecuritySetting();


void APInit();


#endif // AP_H