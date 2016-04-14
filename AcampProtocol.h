#ifndef ACAMPPROTOCOL_H
#define ACAMPPROTOCOL_H
#include "Common.h"


/*Msg Len = 16 byte + PayLoad (4 + len)
 * -----------------------------------------
0        8       16         24         32
|             Preamble                    |
|Version |  Type  |        APID           |
|              Seq_Num                    |
|  Msg Type       |           Msg Len     |
|             *PayLoad                    |
-----------------------------------------*/
typedef struct {
    u32 preamble;
    u8 version;
    u8 type;
    u16 apid;
    u32 seq_num;
    u16 msg_type;
    u16 msg_len;
}acamp_header;

/*PayLoad: len + 4
 * -----------------------------------------
0        8       16         24           32
|Msg Elements Type|      Msg Elements Len |
|          *Msg Elements Data             |
-----------------------------------------*/

typedef struct {
   u16 type;
   u16 len;
   u8* data;
}acamp_element;

acamp_header* make_acamp_header(u32 preamble, u8 version,
                                u8 type, u16 apid, u32 seq_num,
                                u16 msg_type, u16 msg_len);
acamp_element* make_acamp_element(u16 type, u16 len, u8* data);
u8* acamp_encapsulate(acamp_header* header, acamp_element* element[], int ele_num);
int acamp_parse(u8* buf, acamp_header* header, acamp_element* element[], int array_len);
#endif
