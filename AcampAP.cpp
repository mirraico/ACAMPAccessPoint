#include "AcampAP.h"

u16 gAPID = 0;

char gAPName[32];
char gAPDescriptor[128];
u32 gAPIPAddr;
u8 gAPMACAddr[6];
u32 gAPDefaultGateway;

char* gControllerName;
char* gControllerDescriptor;
u32 gControllerIPAddr; 
u8 gControllerMacAddr[6];

char* gSSID;
u8 gChannel;
u8 gHwMode;
u8 gSuppressSSID;
u8 gSecuritySetting;
u8 gWPAVersion;
char* gWPAPasswd;
u8 gWPAKeyManagement;
u8 gWPAPairwise;
u8 gWPARekey[4];
u8 gDefaultWEPKey;
WEPKey gWEPKeys[4];
u8 gMACACLMod;
u8* gMACAcceptList;
u8* gMACDenyList;

u16 gCountryCode;
u8 gEnableDot11d;
u8 gAddSTAMAC[6];
u8 gDelSTAMAC[6];
u32 gStateDescriptor;
