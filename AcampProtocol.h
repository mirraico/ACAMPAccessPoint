#ifndef ACAMPPROTOCOL_H
#define ACAMPPROTOCOL_H
#include "Common.h"


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
}APHeader;
#define HEADER_LEN 16

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
}APElementHeader;
#define ELEMENT_HEADER_LEN 4

typedef struct {
    u8 *msg;
    int offset;
    u16 data_msgType;
} APProtocolMessage;


/* Preamble */
#define PREAMBLE 0x01
#define VERSION 0x01

/* Type */
#define TYPE_CONTROL 0x00

/* Msg Type */
#define MSGTYPE_REGISTER_REQUEST 0x11
#define MSGTYPE_REGISTER_RESPONSE 0x12
#define MSGTYPE_DISCONNET_REQUEST 0x13
#define MSGTYPE_DISCONNET_RESPONSE 0x14
#define MSGTYPE_CONFIGURATION_REQUEST 0x21
#define MSGTYPE_CONFIGURATION_RESPONSE 0x22
#define MSGTYPE_CONFIGURATION_RESET_REQ 0x23
#define MSGTYPE_CONFIGURATION_RESET_RSP 0x24
#define MSGTYPE_STATISTIC_STAT_RP 0x31
#define MSGTYPE_STATISTIC_STAT_QUERY 0x32
#define MSGTYPE_STATISTIC_STAT_REPLY 0x33
#define MSGTYPE_STATION_REQUEST 0x41
#define MSGTYPE_STATION_RESPONSE 0x42

/* Msg Ele Type */
#define MSGELETYPE_RESULT_CODE htons(0x0001)
#define MSGELETYPE_REASON_CODE htons(0x0002)
#define MSGELETYPE_ASSIGNED_APID htons(0x0003)
#define MSGELETYPE_AP_MAC_ADDR htons(0x0101)
#define MSGELETYPE_AP_INET_ADDR htons(0x0102)
#define MSGELETYPE_AP_NAME htons(0x0103)
#define MSGELETYPE_AP_DESCRIPTION htons(0x0104)
#define MSGELETYPE_AP_LOCATION htons(0x0105)
#define MSGELETYPE_AP_BOARD_DATA htons(0x0106)
#define MSGELETYPE_AC_MAC_ADDR htons(0x0201)
#define MSGELETYPE_AC_INET_ADDR htons(0x0202)
#define MSGELETYPE_TIME_STAMP htons(0x0203)
#define MSGELETYPE_WLAN_INFO htons(0x0301)
#define MSGELETYPE_AP_RADIO_INFO htons(0x0302)
#define MSGELETYPE_ANTENNA htons(0x0303)
#define MSGELETYPE_TX_POWER htons(0x0304)
#define MSGELETYPE_MULTI_DOMAIN_CAP htons(0x0305)
#define MSGELETYPE_SUPPORTED_RATES htons(0x0306)
#define MSGELETYPE_ADD_MAC_ACL_ENTRY htons(0x0401)
#define MSGELETYPE_DEL_MAC_ACL_ENTRY htons(0x0402)
#define MSGELETYPE_ADD_STATION htons(0x0403)
#define MSGELETYPE_DEL_STATION htons(0x0404)
#define MSGELETYPE_STATION_EVENT htons(0x0501)

/* Msg Ele Args */
#define RESULTCODE_FAILURE 0
#define RESULTCODE_SUCCESS 1
#define REASONCODE_DEFAULT htons(0x0000)
#define REASONCODE_AC_NO_RESPONE htons(0x0101)
#define REASONCODE_AP_NO_RESPONE htons(0x0102)
#define WLANINFO_QOS_BESTEFFORT 0
#define WLANINFO_QOS_VIDEO 1
#define WLANINFO_QOS_VOICE 2
#define WLANINFO_QOS_BACKGROUND 3
#define WLANINFO_AUTHTYPE_OPENSYSTEM 0
#define WLANINFO_AUTHTYPE_WEP 1
#define WLANINFO_SUPRESS_SSID_NOTSUPRESS 0
#define WLANINFO_SUPRESS_SSID_SUPRESS 1
#define RADIOINFO_SHORTPREAMBLE_NOTSUPPORTED 0
#define RADIOINFO_SHORTPREAMBLE_SUPPORTED 1
#define ANTENNA_DIVERSITY_DISABLED 0
#define ANTENNA_DIVERSITY_ENABLED 1
#define ANTENNA_COMBINER_LEFTSECTORIEZD 0
#define ANTENNA_COMBINER_RIGHTSECTORIEZD 1
#define ANTENNA_COMBINER_OMNI 2
#define ANTENNA_COMBINER_MIMO 3
#define ANTENNA_SELECTION_INTER 1
#define ANTENNA_SELECTION_EXTER 2

#define		AP_CREATE_PROTOCOL_MESSAGE(mess, size)		{ AP_CREATE_OBJECT_SIZE(((mess).msg), (size));		\
                                    AP_ZERO_MEMORY(((mess).msg), (size));						\
                                    (mess).offset = 0; }
#define		AP_FREE_PROTOCOL_MESSAGE(mess)				{ AP_FREE_OBJECT(((mess).msg));								\
                                    (mess).offset = 0; }

void APProtocolStore8(APProtocolMessage *msgPtr, u8 val);
void APProtocolStore16(APProtocolMessage *msgPtr, u16 val);
void APProtocolStore32(APProtocolMessage *msgPtr, u32 val);
void APProtocolStoreStr(APProtocolMessage *msgPtr, char *str);
void APProtocolStoreMessage(APProtocolMessage *msgPtr, APProtocolMessage *msgToStorePtr);
void APProtocolStoreRawBytes(APProtocolMessage *msgPtr, u8 *bytes, int len);

void APProtocolDestroyMsgElemData(void *f);

APBool APAssembleMsgElem(APProtocolMessage *msgPtr, u16 type);
APBool APAsembleMessage(APProtocolMessage **completeMsgPtr,
                         int seqNum, int msgType, APProtocolMessage *msgElems, const int msgElemNum);




#endif
