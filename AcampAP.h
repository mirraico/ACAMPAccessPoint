#ifndef __ACAMPAP_H__
#define __ACAMPAP_H__

#include "Common.h"

extern u16 gAPID;
extern char gAPName[32];
extern char gLocalAddr[20];
extern char gLocalDefaultGateway[20];

__inline__ u16 APGetAPID() { return gAPID; }
__inline__ char* APGetAPName() { return gAPName; }
__inline__ char* APGetLocalAddr() { return gLocalAddr; }
__inline__ char* APGetLocalDefaultGateway() { return gLocalDefaultGateway; }

#endif // ACAMPAP_H

