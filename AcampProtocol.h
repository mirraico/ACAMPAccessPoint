#ifndef __ACAMPPROTOCOL_H__
#define __ACAMPPROTOCOL_H__

#include "Common.h"

extern u16 gAPID;
extern char gAPName[32];
__inline__ u16 APGetAPID() { return gAPID; }
__inline__ char* APGetAPName() { return gAPName; }

/*Msg Len = 16 byte + PayLoad (4 + len)*num byte
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
    u32 seqNum;
    u16 msgType;
    u16 msgLen;
}APHeaderVal;
#define HEADER_LEN 16
#define PREAMBLE 0x00000001
#define VERSION 0x01
#define TYPE_CONTROL 0x00


/*PayLoad: 4 + len
 * -----------------------------------------
0        8       16         24           32
|Msg Elements Type|      Msg Elements Len |
|          *Msg Elements Data             |
-----------------------------------------*/

typedef struct {
   u16 type;
   u16 len;
   u8* data;
}APElementVal;
#define ELEMENT_HEADER_LEN 4


typedef struct {
    u8 *msg;
    int offset;
    u16 type;
}APProtocolMessage;


/* Msg Type */
#define MSGTYPE_REGISTER_REQUEST 0x0101
#define MSGTYPE_REGISTER_RESPONSE 0x0102
#define MSGTYPE_DISCONNET_REQUEST 0x0103
#define MSGTYPE_DISCONNET_RESPONSE 0x0104
#define MSGTYPE_CONFIGURATION_REQUEST 0x0201
#define MSGTYPE_CONFIGURATION_RESPONSE 0x0202
#define MSGTYPE_CONFIGURATION_RESET_REQ 0x0203
#define MSGTYPE_CONFIGURATION_RESET_RSP 0x0204
#define MSGTYPE_STATISTIC_STAT_RP 0x0301
#define MSGTYPE_STATISTIC_STAT_QUERY 0x0302
#define MSGTYPE_STATISTIC_STAT_REPLY 0x0303
#define MSGTYPE_STATION_REQUEST 0x0401
#define MSGTYPE_STATION_RESPONSE 0x0402

/* Msg Ele Type */
#define MSGELETYPE_RESULT_CODE 0x0001
#define MSGELETYPE_REASON_CODE 0x0002
#define MSGELETYPE_ASSIGNED_APID 0x0003
#define MSGELETYPE_AP_MAC_ADDR 0x0101
#define MSGELETYPE_AP_INET_ADDR 0x0102
#define MSGELETYPE_AP_NAME 0x0103
#define MSGELETYPE_AP_DESCRIPTION 0x0104
#define MSGELETYPE_AP_LOCATION 0x0105
#define MSGELETYPE_AP_BOARD_DATA 0x0106
#define MSGELETYPE_AC_MAC_ADDR 0x0201
#define MSGELETYPE_AC_INET_ADDR 0x0202
#define MSGELETYPE_TIME_STAMP 0x0203
#define MSGELETYPE_WLAN_INFO 0x0301
#define MSGELETYPE_AP_RADIO_INFO 0x0302
#define MSGELETYPE_ANTENNA 0x0303
#define MSGELETYPE_TX_POWER 0x0304
#define MSGELETYPE_MULTI_DOMAIN_CAP 0x0305
#define MSGELETYPE_SUPPORTED_RATES 0x0306
#define MSGELETYPE_ADD_MAC_ACL_ENTRY 0x0401
#define MSGELETYPE_DEL_MAC_ACL_ENTRY 0x0402
#define MSGELETYPE_ADD_STATION 0x0403
#define MSGELETYPE_DEL_STATION 0x0404
#define MSGELETYPE_STATION_EVENT 0x0501


typedef enum {
    AP_ENTER_REGISTER,
    AP_ENTER_RUN,
    AP_ENTER_DOWN
} APStateTransition;


#define		AP_CREATE_PROTOCOL_MESSAGE(mess, size)		{ AP_CREATE_OBJECT_SIZE(((mess).msg), (size));\
                                    AP_ZERO_MEMORY(((mess).msg), (size));\
                                    (mess).offset = 0; }
#define		AP_FREE_PROTOCOL_MESSAGE(mess)				{ AP_FREE_OBJECT(((mess).msg));\
                                    (mess).msg = NULL;\
                                    (mess).offset = 0; }
#define	AP_INIT_PROTOCOL_MESSAGE(mess) {(mess).msg = NULL; (mess).offset = 0;}
#define 	AP_CREATE_PROTOCOL_ARRAY_AND_INIT(ar_name, ar_size) 	{\
                                            AP_CREATE_ARRAY(ar_name, ar_size, APProtocolMessage)\
                                            int i;\
                                            for(i=0;i<(ar_size); i++) {\
                                                (ar_name)[i].msg = NULL;\
                                                (ar_name)[i].offset = 0; \
                                            }\
                                        }
#define 	AP_FREE_PROTOCOL_AND_MESSAGE_ARRAY(ar_name, ar_size) 	{\
                                            int i;\
                                            for(i=0;i<(ar_size); i++) {\
                                                AP_FREE_PROTOCOL_MESSAGE((ar_name)[i]);\
                                            }\
                                            AP_FREE_OBJECT(ar_name);\
                                        }


void APProtocolStore8(APProtocolMessage *msgPtr, u8 val);
void APProtocolStore16(APProtocolMessage *msgPtr, u16 val);
void APProtocolStore32(APProtocolMessage *msgPtr, u32 val);
void APProtocolStoreStr(APProtocolMessage *msgPtr, char *str);
void APProtocolStoreMessage(APProtocolMessage *msgPtr, APProtocolMessage *msgToStorePtr);
void APProtocolStoreRawBytes(APProtocolMessage *msgPtr, u8 *bytes, int len);

APBool APAssembleMsgElem(APProtocolMessage *msgPtr, u16 type);
APBool APAssembleMessage(APProtocolMessage *msgPtr,
                         u32 seqNum, u16 msgType, APProtocolMessage msgElems[], const int msgElemNum);
APBool APAssembleControlHeader(APProtocolMessage *controlHdrPtr, APHeaderVal *valPtr);

u8 APProtocolRetrieve8(APProtocolMessage *msgPtr);
u16 APProtocolRetrieve16(APProtocolMessage *msgPtr);
u32 APProtocolRetrieve32(APProtocolMessage *msgPtr);
char *CWProtocolRetrieveStr(APProtocolMessage *msgPtr, int len);
u8 *CWProtocolRetrieveRawBytes(APProtocolMessage *msgPtr, int len);



APBool APAssembleMsgElemAPName(APProtocolMessage *msgPtr);

#endif
