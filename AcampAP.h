#ifndef __ACAMPAP_H__
#define __ACAMPAP_H__

u16 gAPID;
char* gAPName;

u16 APGetAPID();
char* APGetAPName();

__inline__ u16 APGetAPID() { return gAPID; }
__inline__ char* APGetAPName() { return gAPName; }

#endif // ACAMPAP_H

