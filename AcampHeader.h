#ifndef ACAMPHEADER_H
#define ACAMPHEADER_H
#include "Common.h"
/*-----------------------------------------
0        8       16         24         32
|             Preamble                    |
|Version |   Type |        APID           |
|             Seq_Num                     |
|             *PayLoad                    |
-----------------------------------------*/
typedef struct {
    u_int32_t Preamble;
    u_int8_t Version;
    u_int8_t Type;
    u_int16_t APID;
    u_int32_t Seq_Num;
    u_int8_t PayLoad[8];
}APRecv_Header;

/*PayLoad:
 * -----------------------------------------
0        8       16         24           32
|Msg Elements Type|      Msg Elements Len |
|          Msg Elements Data              |
-----------------------------------------*/

typedef struct {
   u_int16_t MsgEle_T;
   u_int16_t MsgEle_Len;
   u_int32_t MsgEle_Value;
}APRecv_Msg;

#endif // ACAMPHEADER_H
