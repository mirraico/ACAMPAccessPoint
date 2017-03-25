#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "common.h"
#include "error.h"
#include "ap.h"


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
#define CURRENT_VERSION 0x03
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

/* general type */
typedef struct {
	u8 *msg;
	int offset;
	u16 type;
}APProtocolMessage;


/* msg type */
#define MSGTYPE_KEEPALIVE_REQUEST 0x0001
#define MSGTYPE_KEEPALIVE_RESPONSE 0x0002
#define MSGTYPE_DISCOVERY_REQUEST 0x0003
#define MSGTYPE_DISCOVERY_RESPONSE 0x0004
#define MSGTYPE_REGISTER_REQUEST 0x0101
#define MSGTYPE_REGISTER_RESPONSE 0x0102
#define MSGTYPE_UNREGISTER_REQUEST 0x0103
#define MSGTYPE_UNREGISTER_RESPONSE 0x0104
#define MSGTYPE_CONFIGURATION_REQUEST 0x0201
#define MSGTYPE_CONFIGURATION_RESPONSE 0x0202
#define MSGTYPE_CONFIGURATION_UPDATE_REQUEST 0x0203
#define MSGTYPE_CONFIGURATION_UPDATE_RESPONSE 0x0204
//#define MSGTYPE_SCAN_REQUEST 0x0301
//#define MSGTYPE_SCAN_RESPONSE 0x0302
//#define MSGTYPE_STATION_REQUEST 0x0303
//#define MSGTYPE_STATION_RESPONSE 0x0304
//#define MSGTYPE_STATISTIC_REQUEST 0x0305
//#define MSGTYPE_STATISTIC_RESPONSE 0x0306
#define MSGTYPE_SYSTEM_REQUEST 0x0307
#define MSGTYPE_SYSTEM_RESPONSE 0x0308

/* msg elem type */
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
//#define MSGELEMTYPE_RETURNED_MSGELEM 0x000e
#define MSGELEMTYPE_CONTROLLER_NEXTSEQ 0x0010
#define MSGELEMTYPE_DESIRED_CONF_LIST 0x0011
#define MSGELEMTYPE_SSID 0x0101
#define MSGELEMTYPE_CHANNEL 0x0102
#define MSGELEMTYPE_HARDWARE_MODE 0x0103
#define MSGELEMTYPE_SUPPRESS_SSID 0x0104
#define MSGELEMTYPE_SECURITY_OPTION 0x0105
#define MSGELEMTYPE_MACFILTER_MODE 0x0106
#define MSGELEMTYPE_MACFILTER_LIST 0x0107
#define MSGELEMTYPE_TX_POWER 0x0108
//#define MSGELEMTYPE_WEP_INFO 0x0201
#define MSGELEMTYPE_WPA_PWD 0x0202
//#define MSGELEMTYPE_WPA_AUTH 0x0203
#define MSGELEMTYPE_ADD_MACFILTER 0x0501
#define MSGELEMTYPE_DEL_MACFILTER 0x0502
#define MSGELEMTYPE_CLEAR_MACFILTER 0x0503
#define MSGELEMTYPE_SYSTEM_COMMAND 0x0401
//#define MSGELEMTYPE_SCANNED_WLAN_INFO 0x0402
//#define MSGELEMTYPE_STATION_INFO 0x0403
//#define MSGELEMTYPE_STATISTICS 0x0404

/* result code */
#define RESULT_SUCCESS 0x0000
#define RESULT_FAILURE 0x0001
//#define RESULT_UNRECOGNIZED_REQ 0x0002

/* reason code */
#define REASON_INVALID_VERSION 0x0101
#define REASON_INSUFFICIENT_RESOURCE 0x0102

/* registered service */
#define REGISTERED_SERVICE_CONF_STA 0x00

/* discovery type */
#define DISCOVERY_TPYE_DISCOVERY 0
#define DISCOVERY_TPYE_STATIC 1
#define DISCOVERY_TPYE_DEFAULT_GATE 2
//#define DISCOVERY_TPYE_DNS 3

/* returned msg elem */
//#define RETERNED_ELEM_UNRECOGNIZED_ELEM 0x0000

/* hardware mode */
#define HWMODE_A 0
#define HWMODE_B 1
#define HWMODE_G 2
#define HWMODE_N 3

/* suppress ssid */
#define SUPPRESS_SSID_DISABLED 0
#define SUPPRESS_SSID_ENABLED 1

/* security option */
#define SECURITY_OPEN 0
//#define SECURITY_WEP 1
#define SECURITY_WPA_WPA2_MIXED 2
#define SECURITY_WPA 3
#define SECURITY_WPA2 4

/* wep type 
#define WEPTYPE_CHAR5 1
#define WEPTYPE_CHAR13 2
#define WEPTYPE_CHAR16 3
#define WEPTYPE_HEX10 4
#define WEPTYPE_HEX26 5
#define WEPTYPE_HEX32 6
*/

/* wpa pairwire cipher 
#define WPA_PAIRWIRECIPHER_TKIP 0
#define WPA_PAIRWIRECIPHER_CCMP 1
#define WPA_PAIRWIRECIPHER_TKIP_CCMP 2
*/

/* mac filter */
#define FILTER_NONE 0
#define FILTER_ACCEPT_ONLY 1
#define FILTER_DENY 2

/* system command */
#define SYSTEM_WLAN_DOWN 0
#define SYSTEM_WLAN_UP 1
#define SYSTEM_WLAN_RESTART 2
#define SYSTEM_NETWORK_RESTART 3
#define SYSTEM_SYSTEM_RESTART 4

/* fsm */
typedef enum {
	AP_ENTER_DOWN,
	AP_ENTER_DISCOVERY,
	AP_ENTER_REGISTER,
	//AP_ENTER_CONFIGURE,
	//AP_ENTER_CONFIGURE_RETRY,
	AP_ENTER_RUN
} APStateTransition;

/**
 * initialize APProtocolMessage structure, without any msg
 * after an APProtocolMessage being created, AP_INIT_PROTOCOL or AP_INIT_PROTOCOL_MESSAGE must be called immediately
 * @param  mess  [a not initialized value]
 */
#define		AP_INIT_PROTOCOL(mess) {\
							(mess).msg = NULL;\
							(mess).offset = 0;\
							(mess).type = 0; }

/**
 * initialize APProtocolMessage structure, including memory allocation and clear, for required size
 * after an APProtocolMessage being created, AP_INIT_PROTOCOL or AP_INIT_PROTOCOL_MESSAGE must be called immediately
 * @param  mess  [a not initialized value]
 * @param  size  [required size]
 */
#define		AP_INIT_PROTOCOL_MESSAGE(mess, size, err) {\
							AP_CREATE_OBJECT_SIZE_ERR(((mess).msg), (size), err);\
							AP_ZERO_MEMORY(((mess).msg), (size));\
							(mess).offset = 0;\
							(mess).type = 0; }

/**
 * free APProtocolMessage structure, including releasing memory
 * before an APProtocolMessage, that is local value, being destoryed, this function must be called
 * @param  mess  [value that need to be freed]
 */
#define		AP_FREE_PROTOCOL_MESSAGE(mess) {\
 							AP_FREE_OBJECT(((mess).msg));\
							(mess).msg = NULL;\
							(mess).offset = 0;\
							(mess).type = 0; }

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
void APParseUnrecognizedMsgElem(APProtocolMessage *msgPtr, int len);
void APParseRepeatedMsgElem(APProtocolMessage *msgPtr, int len);


APBool APParseControllerName(APProtocolMessage *msgPtr, int len, char **valPtr);
APBool APParseControllerDescriptor(APProtocolMessage *msgPtr, int len, char **valPtr);
APBool APParseControllerIPAddr(APProtocolMessage *msgPtr, int len, u32 *valPtr);
APBool APParseControllerMACAddr(APProtocolMessage *msgPtr, int len, u8 *valPtr);
APBool APParseControllerNextSeq(APProtocolMessage *msgPtr, int len, u32 *valPtr);

APBool APAssembleRegisteredService(APProtocolMessage *msgPtr);
APBool APAssembleAPName(APProtocolMessage *msgPtr);
APBool APAssembleAPDescriptor(APProtocolMessage *msgPtr);
APBool APAssembleAPIPAddr(APProtocolMessage *msgPtr);
APBool APAssembleAPMACAddr(APProtocolMessage *msgPtr);
APBool APAssembleDiscoveryType(APProtocolMessage *msgPtr);

APBool APParseResultCode(APProtocolMessage *msgPtr, int len, u16 *valPtr);
APBool APParseReasonCode(APProtocolMessage *msgPtr, int len, u16 *valPtr);
APBool APParseAssignedAPID(APProtocolMessage *msgPtr, int len, u16 *valPtr);
APBool APParseRegisteredService(APProtocolMessage *msgPtr, int len, u8 *valPtr);

APBool APAssembleSSID(APProtocolMessage *msgPtr);
APBool APAssembleChannel(APProtocolMessage *msgPtr);
APBool APAssembleHardwareMode(APProtocolMessage *msgPtr);
APBool APAssembleSuppressSSID(APProtocolMessage *msgPtr);
APBool APAssembleSecurityOption(APProtocolMessage *msgPtr);
APBool APAssembleMACFilterMode(APProtocolMessage *msgPtr);
APBool APAssembleMACFilterList(APProtocolMessage *msgPtr);
APBool APAssembleTxPower(APProtocolMessage *msgPtr);
APBool APAssembleWPAPassword(APProtocolMessage *msgPtr);
//APBool APAssembleWEP(APProtocolMessage *msgPtr);
//APBool APAssembleWPA(APProtocolMessage *msgPtr);

APBool APParseDesiredConfList(APProtocolMessage *msgPtr, int len, u8 **valPtr);
// APBool APParseSSID(APProtocolMessage *msgPtr, int len, char **valPtr);
// APBool APParseChannel(APProtocolMessage *msgPtr, int len, u8 *valPtr);
// APBool APParseHardwareMode(APProtocolMessage *msgPtr, int len, u8 *valPtr);
// APBool APParseSuppressSSID(APProtocolMessage *msgPtr, int len, u8 *valPtr);
// APBool APParseSecurityOption(APProtocolMessage *msgPtr, int len, u8 *valPtr);
//APBool APParseWEP(APProtocolMessage *msgPtr, int len, APWEP *valPtr);
//APBool APParseWPA(APProtocolMessage *msgPtr, int len, APWPA *valPtr);

// APBool APAssembleResultCode(APProtocolMessage *msgPtr, u16 code);

#endif
