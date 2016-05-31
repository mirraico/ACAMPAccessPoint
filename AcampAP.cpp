#include "AcampAP.h"


u16 gAPID;
char gLocalAddr[20];
char gLocalDefaultGateway[20];

char gAPName[32];
char gAPDescriptor[128];
u8 gAPIPAddr[4];
u8 gAPMacAddr[6];

u8 gCntlName[32];
u8 gCntlDescriptor[128];
u8 gCntlIPAddr[4]; 
u8 gCntlMacAddr[6];

u8 gSSID[32];
u8 gChannel;
u8 gHwMode;
u8 gSuppressSSID;
u8 gSecuritySetting;
u8 gWPAVersion;
u8 gWPAPasswd[63];
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
