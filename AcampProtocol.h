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
    u32 seq_num;
    u16 msg_type;
    u16 msg_len;
}acamp_header;
//define HEADER_LEN 16

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
}acamp_element;
#define ELEMENT_LEN 4

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
#define NMSGTYPE_CONFIGURATION_RESET_REQ 0x23
#define MSGTYPE_CONFIGURATION_RESET_RSP 0x24
#define MSGTYPE_STATISTIC_STAT_RP 0x31
#define MSGTYPE_STATISTIC_STAT_QUERY 0x32
#define MSGTYPE_STATISTIC_STAT_REPLY 0x33
#define MSGTYPE_STAT_REQUEST 0x41
#define MSGTYPE_STAT_RESPONSE 0x42

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

acamp_header* make_acamp_header(u32 preamble, u8 version,
                                u8 type, u16 apid, u32 seq_num,
                                u16 msg_type, u16 msg_len);
acamp_element* make_acamp_element(u16 type, u16 len, u8* data);
u8* acamp_encapsulate(acamp_header* header, acamp_element* element[], int ele_num);
int acamp_parse(u8* buf, acamp_header* header, acamp_element* element[], int array_len);
#endif
