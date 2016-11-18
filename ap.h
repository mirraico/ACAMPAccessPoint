#ifndef __AP_H__
#define __AP_H__

#include "common.h"

extern u32 gSeqNum;
extern u32 gWaitSeqNum;
extern u32 gControllerSeqNum;

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

extern int gHdSysLogModules;
extern int gHdSysLogLevel;
extern int gHdStdoutModules;
extern int gHdStdoutLevel;

extern char* gSSID;
extern u8 gChannel;
extern u8 gHardwareMode;
extern u8 gSuppressSSID;
extern u8 gSecurityOption;

/* wep */
typedef struct {
	u8 default_key;
	u8 key0_type;
	char* key0;
	u8 key1_type;
	char* key1;
	u8 key2_type;
	char* key2;
	u8 key3_type;
	char* key3;
} APWEP;

extern int WEP_LEN[7];

/* wpa/wpa2 */
typedef struct {
	u8 version;
	u8 pairwire_cipher;
	char* password;
	u32 group_rekey;
} APWPA;

extern APWEP gWEP;
extern APWPA gWPA;

#define APSeqNumIncrement()         gSeqNum++
#define APWaitSeqNumIncrement()     gWaitSeqNum++

extern u32 APGetSeqNum();
extern u32 APGetWaitSeqNum();

extern u16 APGetAPID();
extern void APSetAPID(u16 apid);
extern u8 APGetDiscoveryType();
extern u8 APGetRegisteredService();

extern u32 APGetStaticControllerIPAddr(); 

extern char* APGetControllerName();
extern char* APGetControllerDescriptor();
extern u32 APGetControllerIPAddr(); 
extern u8* APGetControllerMACAddr();

extern char* APGetAPName();
extern char* APGetAPDescriptor();
extern u32 APGetAPIPAddr();
extern u8* APGetAPMACAddr();
extern u32 APGetAPDefaultGateway();

extern int APGetHdSysLogModules();
extern int APGetHdSysLogLevel();
extern int APGetHdStdoutModules();
extern int APGetHdStdoutLevel();

extern char* APGetSSID();
extern u8 APGetChannel();
extern u8 APGetHardwareMode();
extern u8 APGetSuppressSSID();
extern u8 APGetSecurityOption();

extern APWEP* APGetWEP();
extern APWPA* APGetWPA();

void APInit();


#endif // AP_H