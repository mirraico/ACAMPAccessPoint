#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "common.h"
#include "error.h"


#define PROTOCOL_PORT 6606

/*
msg
msg len = 16 byte + payload(4 + len)*num byte
-------------------------------------------
0        8        16          24         32
|Version |  Type  |        APID           |
|              Seq Num                    |
|  Msg Type       |           Msg Len     |
|              Reserved                   |
|              *PayLoad                   |
-------------------------------------------
*/

typedef struct {
	u8 version;
	u8 type;
	u16 apid;
	u32 seqNum;
	u16 msgType;
	u16 msgLen;
} APHeaderVal;
#define HEADER_LEN 16
#define CURRENT_VERSION 0x02
#define TYPE_CONTROL 0x00


/*
msg_elem
msg_elem len =  4 + datalen
-------------------------------------------
0        8        16         24          32
|Msg Elements Type|    Msg Elements Len   |
|          *Msg Elements Data             |
-------------------------------------------
*/

typedef struct {
	u16 type;
	u16 len;
	u8* data;
} APElementVal;
#define ELEMENT_HEADER_LEN 4

/*
general type
*/
typedef struct {
	u8 *msg;
	int offset;
	u16 type;
}APProtocolMessage;


/*
msg type 
*/
#define MSGTYPE_KEEPALIVE_REQUEST 0x0001
#define MSGTYPE_KEEPALIVE_RESPONSE 0x0002
#define MSGTYPE_DISCOVERY_REQUEST 0x0003
#define MSGTYPE_DISCOVERY_RESPONSE 0x0004
#define MSGTYPE_REGISTER_REQUEST 0x0101
#define MSGTYPE_REGISTER_RESPONSE 0x0102
#define MSGTYPE_UNREGISTER_REQUEST 0x0103
#define MSGTYPE_UNREGISTER_RESPONSE 0x0104
#define MSGTYPE_CONFIGURATION_UPDATE_REQUEST 0x0201
#define MSGTYPE_CONFIGURATION_UPDATE_RESPONSE 0x0202
#define MSGTYPE_CONFIGURATION_REQUEST 0x0203
#define MSGTYPE_CONFIGURATION_RESPONSE 0x0204

/*
msg elem type
*/
#define MSGELEMTYPE_RESULT_CODE 0x0001
#define MSGELEMTYPE_REASON_CODE 0x0002
#define MSGELEMTYPE_ASSIGNED_APID 0x0003
#define MSGELEMTYPE_DISCOVERY_TYPE 0x0004
#define MSGELEMTYPE_REGISTERED_SERVICE 0x0005
#define MSGELEMTYPE_CONTROLLER_NAME 0x0006
#define MSGELEMTYPE_CONTROLLER_DESCRIPTOR 0x0007
#define MSGELEMTYPE_CONTROLLER_IP_ADDR 0x0008
#define MSGELEMTYPE_CONTROLLER_MAC_ADDR 0x0009
#define MSGELEMTYPE_AP_NAME 0x000a
#define MSGELEMTYPE_AP_DESCRIPTOR 0x000b
#define MSGELEMTYPE_AP_IP_ADDR 0x000c
#define MSGELEMTYPE_AP_MAC_ADDR 0x000d
#define MSGELEMTYPE_RETURNED_MSGELE 0x000e
#define MSGELEMTYPE_SSID 0x0101
#define MSGELEMTYPE_CHANNEL 0x0102
#define MSGELEMTYPE_HARDWARE_MODE 0x0103
#define MSGELEMTYPE_SUPPRESS_SSID 0x0104
#define MSGELEMTYPE_SECURITY_SETTING 0x0105
#define MSGELEMTYPE_WPA_VERSION 0x0201
#define MSGELEMTYPE_WPA_PASSPHRASE 0x0202
#define MSGELEMTYPE_WPA_KEY_MANAGEMENT 0x0203
#define MSGELEMTYPE_WPA_PAIRWISE 0x0204
#define MSGELEMTYPE_GROUP_REKEY 0x0205
#define MSGELEMTYPE_DEFAULT_KEY 0x0301
#define MSGELEMTYPE_WEP_KEY 0x0302

/*
fsm
*/
typedef enum {
	AP_ENTER_DOWN,
	AP_ENTER_DISCOVERY,
	AP_ENTER_REGISTER,
	AP_ENTER_CONFIGURE,
	AP_ENTER_CONFIGURE_RETRY,
	AP_ENTER_RUN
} APStateTransition;

/**
 * initialize APProtocolMessage structure, without any msg
 * after an APProtocolMessage being created, AP_INIT_PROTOCOL or AP_INIT_PROTOCOL_MESSAGE must be called immediately
 * @param  mess  [a not initialized value]
 */
#define		AP_INIT_PROTOCOL(mess) {\
							(mess).msg = NULL;\
							(mess).offset = 0; }

/**
 * initialize APProtocolMessage structure, including memory allocation and clear, for required size
 * after an APProtocolMessage being created, AP_INIT_PROTOCOL or AP_INIT_PROTOCOL_MESSAGE must be called immediately
 * @param  mess  [a not initialized value]
 * @param  size  [required size]
 */
#define		AP_INIT_PROTOCOL_MESSAGE(mess, size, err) {\
							AP_CREATE_OBJECT_SIZE_ERR(((mess).msg), (size), err);\
							AP_ZERO_MEMORY(((mess).msg), (size));\
							(mess).offset = 0; }

/**
 * free APProtocolMessage structure, including releasing memory
 * before an APProtocolMessage, that is local value, being destoryed, this function must be called
 * @param  mess  [value that need to be freed]
 */
#define		AP_FREE_PROTOCOL_MESSAGE(mess) {\
 							AP_FREE_OBJECT(((mess).msg));\
							(mess).msg = NULL;\
							(mess).offset = 0; }

/**
 * create a array of APProtocolMessage structure, but not include memory allocation. commonly used in msg element
 * @param  ar_name  [name for array]
 * @param  ar_size  [required size for array]
 */
#define 	AP_CREATE_PROTOCOL_ARRAY(ar_name, ar_size, err) {\
							AP_CREATE_ARRAY_ERR(ar_name, ar_size, APProtocolMessage, err)\
							int i;\
							for(i=0;i<(ar_size); i++) {\
								(ar_name)[i].msg = NULL;\
								(ar_name)[i].offset = 0;\
							} }

/**
 * free a array of APProtocolMessage structure, it's worth mentioning that this action includes releasing memory
 * @param  ar_name  [name for array]
 * @param  ar_size  [size of array]
 */
#define 	AP_FREE_ARRAY_AND_PROTOCOL_MESSAGE(ar_name, ar_size) {\
							int i;\
							for(i=0;i<(ar_size); i++) {\
								AP_FREE_PROTOCOL_MESSAGE((ar_name)[i]);\
							}\
							AP_FREE_OBJECT(ar_name); }


void APProtocolStore8(APProtocolMessage *msgPtr, u8 val);
void APProtocolStore16(APProtocolMessage *msgPtr, u16 val);
void APProtocolStore32(APProtocolMessage *msgPtr, u32 val);
void APProtocolStoreStr(APProtocolMessage *msgPtr, char *str);
void APProtocolStoreMessage(APProtocolMessage *msgPtr, APProtocolMessage *msgToStorePtr);
void APProtocolStoreRawBytes(APProtocolMessage *msgPtr, u8 *bytes, int len);
void APProtocolStoreReserved(APProtocolMessage *msgPtr, int reservedLen);

APBool APAssembleMsgElem(APProtocolMessage *msgPtr, u16 type);
APBool APAssembleControlMessage(APProtocolMessage *msgPtr, u16 apid, u32 seqNum,
						 u16 msgType, APProtocolMessage *msgElems, int msgElemNum);
APBool APAssembleControlHeader(APProtocolMessage *controlHdrPtr, APHeaderVal *valPtr);

u8 APProtocolRetrieve8(APProtocolMessage *msgPtr);
u16 APProtocolRetrieve16(APProtocolMessage *msgPtr);
u32 APProtocolRetrieve32(APProtocolMessage *msgPtr);
char *APProtocolRetrieveStr(APProtocolMessage *msgPtr, int len);
u8 *APProtocolRetrieveRawBytes(APProtocolMessage *msgPtr, int len);
void APProtocolRetrieveReserved(APProtocolMessage *msgPtr, int reservedLen);

APBool APParseControlHeader(APProtocolMessage *msgPtr, APHeaderVal *valPtr);
void APParseFormatMsgElem(APProtocolMessage *msgPtr, u16 *type, u16 *len);


APBool APParseControllerName(APProtocolMessage *msgPtr, int len, char **valPtr);
APBool APParseControllerDescriptor(APProtocolMessage *msgPtr, int len, char **valPtr);
APBool APParseControllerIPAddr(APProtocolMessage *msgPtr, int len, u32 *valPtr);

#endif
