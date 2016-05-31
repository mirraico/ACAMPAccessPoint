#ifndef __ACAMPPROTOCOL_H__
#define __ACAMPPROTOCOL_H__

#include "Common.h"
#include "AcampAP.h"


/*Msg Len = 16 byte + PayLoad (4 + len)*num byte
 * -----------------------------------------
0        8       16         24         32
|Version |  Type  |        APID           |
|              Seq_Num                    |
|  Msg Type       |           Msg Len     |
|             *PayLoad                    |
-----------------------------------------*/
typedef struct {
	u8 version;
	u8 type;
	u16 apid;
	u32 seqNum;
	u16 msgType;
	u16 msgLen;
}APHeaderVal;
#define HEADER_LEN 12
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
#define MSGTYPE_KEEP_ALIVE_REQUEST 0x01
#define MSGTYPE_KEEP_ALIVE_REAPONSE 0X02
#define MSGTYPE_DISCOVER_REQUEST 0x03
#define MSGTYPE_DISCOVER_RESPONSE 0x04
#define MSGTYPE_REGISTER_REQUEST 0x11
#define MSGTYPE_REGISTER_REAPONSE 0x12
#define MSGTYPE_UNREGISTER_REQUEST 0x13
#define MSGTYPE_UNREGISTER_REAPONSE 0x14
#define MSGTYPE_CONFIGURATION_REQUEST  0x21
#define MSGTYPE_CONFIGURATION_DELIVER 0x22
#define MSGTYPE_CONFIGURATION_REPORT 0x23
#define MSGTYPE_STATION_CONFIGURATION_DELIVER 0x24
#define MSGTYPE_STATISTIC_REQUEST 0x31
#define MSGTYPE_STATISTIC_REPORT 0x32
#define MSGTYPE_STATE_REQUEST 0x41
#define MSGTYPE_STATE_REPORT 0x42

/* Msg Ele Type */
#define MSGELETYPE_RESULT_CODE 0x0001
#define MSGELETYPE_REASON_CODE  0x0002
#define MSGELETYPE_ASSIGNED_APID 0x0003
#define MSGELETYPE_CONTROLLER_NAME 0x0004
#define MSGELETYPE_CONTROLLER_DESCRIPTOR 0x0005
#define MSGELETYPE_CONTROLLER_IP_ADDRESS 0x0006
#define MSGELETYPE_CONTROLLER_MAC_ADDRESS 0x0007
#define MSGELETYPE_AP_NAME 0x0008
#define MSGELETYPE_AP_DESCRIPTOR  0x0009
#define MSGELETYPE_AP_IP_ADDRESS 0x000a
#define MSGELETYPE_AP_MAC_ADDRESS 0x000b
#define MSGELETYPE_SSID 0x0101
#define MSGELETYPE_CHANNEL 0x0102
#define MSGELETYPE_HW_MODE 0x0103
#define MSGELETYPE_SUPPRESS_SSID 0x0104
#define MSGELETYPE_SECURITY_SETTING 0x0105
#define MSGELETYPE_WPA_VERSION 0x0201
#define MSGELETYPE_WPA_PASSPHRASE 0x0202
#define MSGELETYPE_WPA_KEY_MANAGEMENT 0x0203
#define MSGELETYPE_WPA_PAIRWISE 0x0204
#define MSGELETYPE_WPA_GROUP_REKEY 0x0205
#define MSGELETYPE_WEP_DEFAULT_KEY  0x0301
#define MSGELETYPE_WEP_KEY 0x0302
#define MSGELETYPE_MAC_ACL_MODE 0x0401
#define MSGELETYPE_MAC_ACCEPT_LIST 0x0402
#define MSGELETYPE_MAC_DENY_LIST 0x0403
#define MSGELETYPE_COUNTRY_CODE  0x0501
#define MSGELETYPE_ENABLE_DOT11D  0x0502
#define MSGELETYPE_ADD_STATION 0x0601
#define MSGELETYPE_DELETE_STATION 0x0602
#define MSGELETYPE_STATES_DESCRIPTOR 0x0701
#define MSGELETYPE_TIMMERS 0x0702

typedef enum {
	AP_DISCOVERY,
	AP_REGISTER,
	AP_RUN,
	AP_DOWN
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
void APProtocolStoreVoid(APProtocolMessage *msgPtr);

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
APBool APAssembleMsgElemAPBoardData(APProtocolMessage *msgPtr);
APBool APAssembleMsgElemAPDescriptor(APProtocolMessage *msgPtr);
APBool APAssembleMsgElemAPRadioInformation(APProtocolMessage *msgPtr) ;

#endif
