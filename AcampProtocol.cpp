#include "AcampProtocol.h"

void APProtocolStore8(APProtocolMessage *msgPtr, u8 val)
{
	AP_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 1);
	(msgPtr->offset) += 1;
}

void APProtocolStore16(APProtocolMessage *msgPtr, u16 val)
{
	val = htons(val);
	AP_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 2);
	(msgPtr->offset) += 2;
}

void APProtocolStore32(APProtocolMessage *msgPtr, u32 val)
{
	val = htonl(val);
	AP_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 4);
	(msgPtr->offset) += 4;
}

void APProtocolStoreStr(APProtocolMessage *msgPtr, char *str)
{
	int len = strlen(str);
	AP_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), str, len);
	(msgPtr->offset) += len;
}

void APProtocolStoreMessage(APProtocolMessage *msgPtr, APProtocolMessage *msgToStorePtr)
{
	AP_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), msgToStorePtr->msg, msgToStorePtr->offset);
	(msgPtr->offset) += msgToStorePtr->offset;
}

void APProtocolStoreRawBytes(APProtocolMessage *msgPtr, u8 *bytes, int len)
{
	AP_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), bytes, len);
	(msgPtr->offset) += len;
}

void APProtocolStoreVoid(APProtocolMessage *msgPtr)
{
	(msgPtr->offset) += 0;
}

APBool APAssembleMsgElem(APProtocolMessage *msgPtr, u16 type)
{
	APProtocolMessage completeMsg;

	if(msgPtr == NULL) return AP_FALSE;
	AP_CREATE_PROTOCOL_MESSAGE(completeMsg, ELEMENT_HEADER_LEN+(msgPtr->offset));

	APProtocolStore16(&completeMsg, type);
	APProtocolStore16(&completeMsg, msgPtr->offset);
	APProtocolStoreMessage(&completeMsg, msgPtr);

	AP_FREE_PROTOCOL_MESSAGE(*msgPtr);

	msgPtr->msg = completeMsg.msg;
	msgPtr->offset = completeMsg.offset;
	msgPtr->type = type;

	return AP_TRUE;
}

APBool APAssembleMessage(APProtocolMessage *msgPtr,
						 u32 seqNum, u16 msgType, APProtocolMessage *msgElems, const int msgElemNum)
{
	APProtocolMessage controlHdr, completeMsg;
	int msgElemsLen = 0;
	APHeaderVal controlHdrVal;

	if(msgPtr == NULL || (msgElems == NULL && msgElemNum > 0))
		return AP_FALSE;

	for(int i = 0; i < msgElemNum; i++) msgElemsLen += msgElems[i].offset;

	controlHdrVal.version = VERSION;
	controlHdrVal.type = TYPE_CONTROL;
	controlHdrVal.apid = APGetAPID();
	controlHdrVal.seqNum = seqNum;
	controlHdrVal.msgType = msgType;
	controlHdrVal.msgLen = HEADER_LEN + msgElemsLen;

	if(!(APAssembleControlHeader(&controlHdr, &controlHdrVal))) {
		AP_FREE_PROTOCOL_MESSAGE(controlHdr);
		AP_FREE_PROTOCOL_AND_MESSAGE_ARRAY(msgElems, msgElemNum);
		return AP_FALSE;
	}

	completeMsg.type = msgType;
	AP_CREATE_PROTOCOL_MESSAGE(completeMsg, controlHdr.offset + msgElemsLen);
	APProtocolStoreMessage(&completeMsg, &controlHdr);
	AP_FREE_PROTOCOL_MESSAGE(controlHdr);

	for(int i = 0; i < msgElemNum; i++) {
		APProtocolStoreMessage(&completeMsg, &(msgElems[i]));
	}
	AP_FREE_PROTOCOL_AND_MESSAGE_ARRAY(msgElems, msgElemNum);

	AP_FREE_PROTOCOL_MESSAGE(*msgPtr);
	msgPtr->msg = completeMsg.msg;
	msgPtr->offset = completeMsg.offset;
	msgPtr->type = msgType;

	return AP_TRUE;
}

APBool APAssembleControlHeader(APProtocolMessage *controlHdrPtr, APHeaderVal *valPtr)
{
	if(controlHdrPtr == NULL || valPtr == NULL) return AP_FALSE;

	AP_CREATE_PROTOCOL_MESSAGE(*controlHdrPtr, HEADER_LEN);

	APProtocolStore8(controlHdrPtr, valPtr->version);
	APProtocolStore8(controlHdrPtr, valPtr->type);
	APProtocolStore16(controlHdrPtr, valPtr->apid);
	APProtocolStore32(controlHdrPtr, valPtr->seqNum);
	APProtocolStore16(controlHdrPtr, valPtr->msgType);
	APProtocolStore16(controlHdrPtr, valPtr->msgLen);

	return AP_TRUE;
}

u8 APProtocolRetrieve8(APProtocolMessage *msgPtr) 
{
	u8 val;

	AP_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 1);
	(msgPtr->offset) += 1;

	return val;
}

u16 APProtocolRetrieve16(APProtocolMessage *msgPtr) 
{
	u16 val;

	AP_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 2);
	(msgPtr->offset) += 2;

	return ntohs(val);
}

u32 APProtocolRetrieve32(APProtocolMessage *msgPtr) 
{
	u32 val;

	AP_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 4);
	(msgPtr->offset) += 4;

	return ntohl(val);
}

char* APProtocolRetrieveStr(APProtocolMessage *msgPtr, int len) 
{
	u8* str;

	AP_CREATE_OBJECT_SIZE(str, (len+1));

	AP_COPY_MEMORY(str, &((msgPtr->msg)[(msgPtr->offset)]), len);
	str[len] = '\0';
	(msgPtr->offset) += len;

	return (char*)str;
}

u8* APProtocolRetrieveRawBytes(APProtocolMessage *msgPtr, int len) 
{
	u8* bytes;

	AP_CREATE_OBJECT_SIZE(bytes, len);

	AP_COPY_MEMORY(bytes, &((msgPtr->msg)[(msgPtr->offset)]), len);
	(msgPtr->offset) += len;

	return bytes;
}

APBool APParseControlHeader(APProtocolMessage *controlHdrPtr, APHeaderVal *valPtr) 
{
	if(controlHdrPtr == NULL|| valPtr == NULL) return AP_FALSE;
	
	valPtr->version = APProtocolRetrieve8(controlHdrPtr);
	valPtr->type = APProtocolRetrieve8(controlHdrPtr);
	valPtr->apid = APProtocolRetrieve16(controlHdrPtr);
	valPtr->seqNum = APProtocolRetrieve32(controlHdrPtr);
	valPtr->msgType = APProtocolRetrieve16(controlHdrPtr);
	valPtr->msgLen = APProtocolRetrieve16(controlHdrPtr);
	
	return AP_TRUE;
}

APBool APParseFormatMsgElement(APProtocolMessage *msgPtr, u16 *type, u16 *len)
{
	if(len == NULL || type == NULL) return AP_FALSE;
	*type = APProtocolRetrieve16(msgPtr);
	*len = APProtocolRetrieve16(msgPtr);
	if(*len == 0) return AP_FALSE;
	return AP_TRUE;
}

APBool APAssembleMsgElemAPName(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return AP_FALSE;

	char *name = APGetAPName();
	AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, strlen(name));

	APProtocolStoreStr(msgPtr, name);
	return APAssembleMsgElem(msgPtr, MSGELETYPE_AP_NAME);
}

APBool APAssembleMsgElemAPDescriptor(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return AP_FALSE;
	
	char *descriptor = APGetAPDescriptor();
	AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, strlen(descriptor));
	
	APProtocolStoreStr(msgPtr, descriptor);
	return APAssembleMsgElem(msgPtr, MSGELETYPE_AP_DESCRIPTOR);
}

APBool APAssembleMsgElemAPIPAddr(APProtocolMessage *msgPtr)
{
	if(msgPtr == NULL) return AP_FALSE;
	
	u32 ipAddr = APGetAPIPAddr();
	AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4);
	
	APProtocolStore32(msgPtr, ipAddr);
	return APAssembleMsgElem(msgPtr, MSGELETYPE_AP_IP_ADDRESS);
}

APBool APAssembleMsgElemAPMACAddr(APProtocolMessage *msgPtr)
{
	if(msgPtr == NULL) return AP_FALSE;
	
	u8* macAddr = APGetAPMACAddr();
	AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, 6);
	
	for(int i = 0; i < 6; i ++) {
		APProtocolStore8(msgPtr, macAddr[i]);
	}
	return APAssembleMsgElem(msgPtr, MSGELETYPE_AP_MAC_ADDRESS);
}

APBool APAssembleMsgElemSSID(APProtocolMessage *msgPtr)
{
    if(msgPtr == NULL) return AP_FALSE;

    char *ssid = APGetSSID();
    AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, strlen(ssid));

    APProtocolStoreStr(msgPtr, ssid);
    return APAssembleMsgElem(msgPtr, MSGELETYPE_SSID);
}

APBool APAssembleMsgElemChannel(APProtocolMessage *msgPtr)
{
    if(msgPtr == NULL) return AP_FALSE;

    u8 channel = APGetChannel();
    AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, sizeof(channel));

    APProtocolStore8(msgPtr, channel);
    return APAssembleMsgElem(msgPtr, MSGELETYPE_CHANNEL);
}

APBool APAssembleMsgElemHardwareMode(APProtocolMessage *msgPtr)
{
    if(msgPtr == NULL) return AP_FALSE;

    u8 hwMode = APGetHwMode();
    AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, sizeof(hwMode));

    APProtocolStore8(msgPtr, hwMode);
    return APAssembleMsgElem(msgPtr, MSGELETYPE_HW_MODE);
}

APBool APAssembleMsgElemSuppressSSID(APProtocolMessage *msgPtr)
{
    if(msgPtr == NULL) return AP_FALSE;

    u8 suppressSSID = APGetSuppressSSID();
    AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, sizeof(suppressSSID));

    APProtocolStore8(msgPtr, suppressSSID);
    return APAssembleMsgElem(msgPtr, MSGELETYPE_SUPPRESS_SSID);
}

APBool APAssembleMsgElemSecuritySetting(APProtocolMessage *msgPtr)
{
    if(msgPtr == NULL) return AP_FALSE;

    u8 securitySetting = APGetSecuritySetting();
    AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, sizeof(securitySetting));

    APProtocolStore8(msgPtr, securitySetting);
    return APAssembleMsgElem(msgPtr, MSGELETYPE_SECURITY_SETTING);
}

APBool APAssembleMsgElemWPAVersion(APProtocolMessage *msgPtr)
{
    if(msgPtr == NULL) return AP_FALSE;

    u8 wpaVersion = APGetWPAVersion();
    AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, sizeof(wpaVersion));

    APProtocolStore8(msgPtr, wpaVersion);
    return APAssembleMsgElem(msgPtr, MSGELETYPE_WPA_VERSION);
}

APBool APAssembleMsgElemWPAPassphrase(APProtocolMessage *msgPtr)
{
    if(msgPtr == NULL) return AP_FALSE;

    char *wpaPasswd = APGetWPAPasswd();
    AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, strlen(wpaPasswd));

    APProtocolStoreStr(msgPtr, wpaPasswd);
    return APAssembleMsgElem(msgPtr, MSGELETYPE_WPA_PASSPHRASE);
}

APBool APAssembleMsgElemWPAKeyManagement(APProtocolMessage *msgPtr)
{
    if(msgPtr == NULL) return AP_FALSE;

    u8 wpaKeyManagement = APGetWPAKeyManagement();
    AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, sizeof(wpaKeyManagement));

    APProtocolStore8(msgPtr, wpaKeyManagement);
    return APAssembleMsgElem(msgPtr, MSGELETYPE_WPA_KEY_MANAGEMENT);
}

APBool APAssembleMsgElemWPAPairwise(APProtocolMessage *msgPtr)
{
    if(msgPtr == NULL) return AP_FALSE;

    u8 wpaPairwise = APGetWPAPairwise();
    AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, sizeof(wpaPairwise));

    APProtocolStore8(msgPtr, wpaPairwise);
    return APAssembleMsgElem(msgPtr, MSGELETYPE_WPA_PAIRWISE);
}



APBool APParseMsgElemControllerName(APProtocolMessage *msgPtr, int elemLen) 
{
	if(msgPtr == NULL) return AP_FALSE;
	if(gControllerName != NULL) AP_FREE_OBJECT(gControllerName);
	gControllerName = APProtocolRetrieveStr(msgPtr, elemLen);
	return AP_TRUE;
}

APBool APParseMsgElemControllerDescriptor(APProtocolMessage *msgPtr, int elemLen) 
{
	if(msgPtr == NULL) return AP_FALSE;
	if(gControllerDescriptor != NULL) AP_FREE_OBJECT(gControllerDescriptor);
	gControllerDescriptor = APProtocolRetrieveStr(msgPtr, elemLen);
	return AP_TRUE;
}

APBool APParseMsgElemControllerIPAddr(APProtocolMessage *msgPtr, int elemLen) 
{
	if(msgPtr == NULL) return AP_FALSE;
	if(elemLen != 4) return AP_FALSE;
	gControllerIPAddr = APProtocolRetrieve32(msgPtr);
	return AP_TRUE;
}

APBool APParseMsgElemControllerMACAddr(APProtocolMessage *msgPtr, int elemLen) 
{
	if(msgPtr == NULL) return AP_FALSE;
	if(elemLen != 6) return AP_FALSE;
	for(int i = 0; i < 6; i++) {
		gControllerMacAddr[i] = APProtocolRetrieve8(msgPtr);
	}
	return AP_TRUE;
}

APBool APParseMsgElemResultCode(APProtocolMessage *msgPtr, int elemLen, int *result)
{
    if(msgPtr == NULL) return AP_FALSE;
    if(elemLen != 2) return AP_FALSE;
    *result = APProtocolRetrieve16(msgPtr);
    return AP_TRUE;
}

APBool APParseMsgElemAssignedAPID(APProtocolMessage *msgPtr, int elemLen)
{
    if(msgPtr == NULL) return AP_FALSE;
    if(elemLen != 2) return AP_FALSE;
    gAPID = APProtocolRetrieve16(msgPtr);
    return AP_TRUE;
}
