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
#define MSGTYPE_DISCOVER_REQUEST 0x03
#define MSGTYPE_DISCOVER_RESPONSE 0x04

/* Msg Ele Type */
#define MSGELETYPE_AP_BOARD_DATA 0x0102
#define MSGELETYPE_AP_DESCRIPTOR 0x0103
#define MSGELETYPE_AP_NAME 0x0104
#define MSGELETYPE_AP_RADIO_INFORMATION 0x030A

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

/*___________________________________________________________________________*/
/* *******************************__PROTYPES__*******************************/
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
