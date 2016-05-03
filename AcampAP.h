#ifndef __ACAMPAP_H__
#define __ACAMPAP_H__

#include "Common.h"

extern u16 gAPID;
extern char gAPName[32];

__inline__ u16 APGetAPID() { return gAPID; }
__inline__ char* APGetAPName() { return gAPName; }

#endif // ACAMPAP_H

