#include "protocol.h"

/**
 * store a 8-bit value into a initialized APProtocolMessage
 * @param msgPtr [a initialized APProtocolMessage and there is space left]
 * @param val    [value you want to store]
 */
void APProtocolStore8(APProtocolMessage *msgPtr, u8 val)
{
	AP_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 1);
	(msgPtr->offset) += 1;
}

/**
 * store a 16-bit value into a initialized APProtocolMessage
 * @param msgPtr [a initialized APProtocolMessage and there is space left]
 * @param val    [value you want to store]
 */
void APProtocolStore16(APProtocolMessage *msgPtr, u16 val)
{
	val = htons(val);
	AP_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 2);
	(msgPtr->offset) += 2;
}

/**
 * store a 32-bit value into a initialized APProtocolMessage
 * @param msgPtr [a initialized APProtocolMessage and there is space left]
 * @param val    [value you want to store]
 */
void APProtocolStore32(APProtocolMessage *msgPtr, u32 val)
{
	val = htonl(val);
	AP_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 4);
	(msgPtr->offset) += 4;
}

/**
 * store a string into a initialized APProtocolMessage
 * @param msgPtr [a initialized APProtocolMessage and there is space left]
 * @param str    [value you want to store, a standard string must end with '\0']
 */
void APProtocolStoreStr(APProtocolMessage *msgPtr, char *str)
{
	int len = strlen(str);
	AP_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), str, len);
	(msgPtr->offset) += len;
}

/**
 * store content in another APProtocolMessage into a initialized APProtocolMessage
 * @param msgPtr        [a initialized APProtocolMessage and there is space left]
 * @param msgToStorePtr [another APProtocolMessage you want to use]
 */
void APProtocolStoreMessage(APProtocolMessage *msgPtr, APProtocolMessage *msgToStorePtr)
{
	AP_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), msgToStorePtr->msg, msgToStorePtr->offset);
	(msgPtr->offset) += msgToStorePtr->offset;
}

/**
 * store raw bytes into a initialized APProtocolMessage
 * @param msgPtr [a initialized APProtocolMessage and there is space left]
 * @param bytes  [value you want to store]
 * @param len    [size of raw bytes]
 */
void APProtocolStoreRawBytes(APProtocolMessage *msgPtr, u8 *bytes, int len)
{
	AP_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), bytes, len);
	(msgPtr->offset) += len;
}

/**
 * store reserved value, that is 0, into a initialized APProtocolMessage
 * @param msgPtr      [a initialized APProtocolMessage and there is space left]
 * @param reservedLen [size of reserved space]
 */
void APProtocolStoreReserved(APProtocolMessage *msgPtr, int reservedLen)
{
	AP_ZERO_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), reservedLen);
	(msgPtr->offset) += reservedLen;
}

/**
 * assemble a formatted msg element by the element content and type
 * @param  msgPtr [the element content, which is preserved in a unformatted APProtocolMessage. after this action, it will transform a formatted msg element]
 * @param  type   [the type of msg elem]
 * @return        [whether the operation is success or not]
 */
APBool APAssembleMsgElem(APProtocolMessage *msgPtr, u16 type)
{
	APProtocolMessage completeMsg;

	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleMsgElem()");
	AP_INIT_PROTOCOL_MESSAGE(completeMsg, ELEMENT_HEADER_LEN+(msgPtr->offset), return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleMsgElem()"););

	APProtocolStore16(&completeMsg, type);
	APProtocolStore16(&completeMsg, msgPtr->offset);
	APProtocolStoreMessage(&completeMsg, msgPtr);

	AP_FREE_PROTOCOL_MESSAGE(*msgPtr);

	msgPtr->msg = completeMsg.msg;
	msgPtr->offset = completeMsg.offset;
	msgPtr->type = type;

	return AP_TRUE;
}

/**
 * assemble a formatted msg that can be send
 * @param  msgPtr     [a APAssembleMessage type value, which will output a formatted msg]
 * @param  apid       [apid that will be used in message header]
 * @param  seqNum     [seq num that will be used in message header]
 * @param  msgType    [the type of msg]
 * @param  msgElems   [a array of msgElems, which will be used in msg]
 * @param  msgElemNum [count of msgElems]
 * @return            [whether the operation is success or not]
 */
APBool APAssembleControlMessage(APProtocolMessage *msgPtr, u16 apid, u32 seqNum,
						 u16 msgType, APProtocolMessage *msgElems, int msgElemNum)
{
	APProtocolMessage controlHdr, completeMsg;
	int msgElemsLen = 0, i;
	APHeaderVal controlHdrVal;
	AP_INIT_PROTOCOL(controlHdr);

	if(msgPtr == NULL || (msgElems == NULL && msgElemNum > 0))
		return APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleControlMessage()");

	for(i = 0; i < msgElemNum; i++) msgElemsLen += msgElems[i].offset;

	controlHdrVal.version = CURRENT_VERSION;
	controlHdrVal.type = TYPE_CONTROL;
	controlHdrVal.apid = apid;
	controlHdrVal.seqNum = seqNum;
	controlHdrVal.msgType = msgType;
	controlHdrVal.msgLen = HEADER_LEN + msgElemsLen;

	if(!(APAssembleControlHeader(&controlHdr, &controlHdrVal))) {
		AP_FREE_PROTOCOL_MESSAGE(controlHdr);
		AP_FREE_ARRAY_AND_PROTOCOL_MESSAGE(msgElems, msgElemNum);
		return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
	}

	AP_INIT_PROTOCOL_MESSAGE(completeMsg, controlHdr.offset + msgElemsLen, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleControlMessage()"););
	APProtocolStoreMessage(&completeMsg, &controlHdr);
	AP_FREE_PROTOCOL_MESSAGE(controlHdr);

	for(i = 0; i < msgElemNum; i++) {
		APProtocolStoreMessage(&completeMsg, &(msgElems[i]));
	}
	AP_FREE_ARRAY_AND_PROTOCOL_MESSAGE(msgElems, msgElemNum);

	AP_FREE_PROTOCOL_MESSAGE(*msgPtr);
	msgPtr->msg = completeMsg.msg;
	msgPtr->offset = completeMsg.offset;
	msgPtr->type = msgType;

	return AP_TRUE;
}

/**
 * assemble a msg header
 * @param  controlHdrPtr [a APAssembleMessage type value, which will output a formatted msg header]
 * @param  valPtr        [header info]
 * @return               [whether the operation is success or not]
 */
APBool APAssembleControlHeader(APProtocolMessage *controlHdrPtr, APHeaderVal *valPtr)
{
	if(controlHdrPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleControlHeader()");

	AP_INIT_PROTOCOL_MESSAGE(*controlHdrPtr, HEADER_LEN, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleControlHeader()"););

	APProtocolStore8(controlHdrPtr, valPtr->version);
	APProtocolStore8(controlHdrPtr, valPtr->type);
	APProtocolStore16(controlHdrPtr, valPtr->apid);
	APProtocolStore32(controlHdrPtr, valPtr->seqNum);
	APProtocolStore16(controlHdrPtr, valPtr->msgType);
	APProtocolStore16(controlHdrPtr, valPtr->msgLen);
	APProtocolStoreReserved(controlHdrPtr, 4);

	return AP_TRUE;
}

/**
 * get a 8-bit value from APProtocolMessage
 * @param  msgPtr [a APProtocolMessage that include value]
 * @return        [u8 value]
 */
u8 APProtocolRetrieve8(APProtocolMessage *msgPtr) 
{
	u8 val;

	AP_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 1);
	(msgPtr->offset) += 1;

	return val;
}

/**
 * get a 16-bit value from APProtocolMessage
 * @param  msgPtr [a APProtocolMessage that include value]
 * @return        [u16 value]
 */
u16 APProtocolRetrieve16(APProtocolMessage *msgPtr) 
{
	u16 val;

	AP_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 2);
	(msgPtr->offset) += 2;

	return ntohs(val);
}

/**
 * get a 32-bit value from APProtocolMessage
 * @param  msgPtr [a APProtocolMessage that include value]
 * @return        [u32 value]
 */
u32 APProtocolRetrieve32(APProtocolMessage *msgPtr) 
{
	u32 val;

	AP_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 4);
	(msgPtr->offset) += 4;

	return ntohl(val);
}

/**
 * get a string from APProtocolMessage
 * @param  msgPtr [a APProtocolMessage that include value]
 * @param  len    [length of string]
 * @return        [a standard string that end with '\0']
 */
char* APProtocolRetrieveStr(APProtocolMessage *msgPtr, int len) 
{
	u8* str;

	AP_CREATE_OBJECT_SIZE_ERR(str, (len+1), return NULL;);

	AP_COPY_MEMORY(str, &((msgPtr->msg)[(msgPtr->offset)]), len);
	str[len] = '\0';
	(msgPtr->offset) += len;

	return (char*)str;
}

/**
 * get raw bytes from APProtocolMessage
 * @param  msgPtr [a APProtocolMessage that include value]
 * @param  len    [size of raw bytes]
 * @return        [an array that include raw bytes]
 */
u8* APProtocolRetrieveRawBytes(APProtocolMessage *msgPtr, int len) 
{
	u8* bytes;

	AP_CREATE_OBJECT_SIZE_ERR(bytes, len, return NULL;);

	AP_COPY_MEMORY(bytes, &((msgPtr->msg)[(msgPtr->offset)]), len);
	(msgPtr->offset) += len;

	return bytes;
}

/**
 * pass reserved value
 * @param msgPtr      [a APProtocolMessage]
 * @param reservedLen [size of reserved space]
 */
void APProtocolRetrieveReserved(APProtocolMessage *msgPtr, int reservedLen) 
{
	(msgPtr->offset) += reservedLen;
}


APBool APParseControlHeader(APProtocolMessage *msgPtr, APHeaderVal *valPtr) 
{	
	if(msgPtr == NULL|| valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APParseControlHeader()");

	valPtr->version = APProtocolRetrieve8(msgPtr);
	valPtr->type = APProtocolRetrieve8(msgPtr);
	valPtr->apid = APProtocolRetrieve16(msgPtr);
	valPtr->seqNum = APProtocolRetrieve32(msgPtr);
	valPtr->msgType = APProtocolRetrieve16(msgPtr);
	valPtr->msgLen = APProtocolRetrieve16(msgPtr);
	APProtocolRetrieveReserved(msgPtr, 4);
	
	return AP_TRUE;
}

void APParseFormatMsgElem(APProtocolMessage *msgPtr, u16 *type, u16 *len)
{
	*type = APProtocolRetrieve16(msgPtr);
	*len = APProtocolRetrieve16(msgPtr);
}

void APParseUnrecognizedMsgElem(APProtocolMessage *msgPtr, int len)
{
	msgPtr->offset += len;
}

void APParseRepeatedMsgElem(APProtocolMessage *msgPtr, int len) 
{
	msgPtr->offset += len;
}

APBool APParseControllerName(APProtocolMessage *msgPtr, int len, char **valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APParseControllerName()");
	
	*valPtr = APProtocolRetrieveStr(msgPtr, len);
	if(valPtr == NULL) return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APParseControllerName()");
	APDebugLog(5, "Parse Controller Name: %s", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		APErrorLog("Message Element Malformed");
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseControllerName()");
	}
	return AP_TRUE;
}

APBool APParseControllerDescriptor(APProtocolMessage *msgPtr, int len, char **valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APParseControllerDescriptor()");
	
	*valPtr = APProtocolRetrieveStr(msgPtr, len);
	if(valPtr == NULL) return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APParseControllerDescriptor()");
	APDebugLog(5, "Parse Controller Descriptor: %s", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		APErrorLog("Message Element Malformed");
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseControllerDescriptor()");
	}
	return AP_TRUE;
}

APBool APParseControllerIPAddr(APProtocolMessage *msgPtr, int len, u32 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APParseControllerIPAddr()");
	
	*valPtr = APProtocolRetrieve32(msgPtr);
	APDebugLog(5, "Parse Controller IP: %u.%u.%u.%u", (u8)(*valPtr >> 24), (u8)(*valPtr >> 16),\
	  (u8)(*valPtr >> 8),  (u8)(*valPtr >> 0));

	if((msgPtr->offset - oldOffset) != len) {
		APErrorLog("Message Element Malformed");
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseControllerIPAddr()");
	}
	return AP_TRUE;
}

APBool APParseControllerMACAddr(APProtocolMessage *msgPtr, int len, u8 *valPtr) 
{	
	int i;
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APParseControllerMACAddr()");
	
	for(i = 0; i < 6; i++) {
		valPtr[i] = APProtocolRetrieve8(msgPtr);
	}
	APDebugLog(5, "Parse Controller MAC: %02x:%02x:%02x:%02x:%02x:%02x", valPtr[0], valPtr[1],\
	 valPtr[2], valPtr[3], valPtr[4], valPtr[5]);

	if((msgPtr->offset - oldOffset) != len) {
		APErrorLog("Message Element Malformed");
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseControllerMACAddr()");
	}
	return AP_TRUE;
}

APBool APAssembleRegisteredService(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleRegisteredService()");
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleRegisteredService()"););
	APDebugLog(5, "Assemble Registered Service: 0x%02x", APGetRegisteredService());
	APProtocolStore8(msgPtr, APGetRegisteredService());

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_REGISTERED_SERVICE);
}

APBool APAssembleAPName(APProtocolMessage *msgPtr) 
{
	char* str;
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleAPName()");
	str = APGetAPName();
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, strlen(str), return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleAPName()"););
	APDebugLog(5, "Assemble AP Name: %s", str);
	APProtocolStoreStr(msgPtr, str);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_AP_NAME);
}

APBool APAssembleAPDescriptor(APProtocolMessage *msgPtr) 
{
	char* str;
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleAPDescriptor()");
	str = APGetAPDescriptor();
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, strlen(str), return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleAPDescriptor()"););
	APDebugLog(5, "Assemble AP Descriptor: %s", str);
	APProtocolStoreStr(msgPtr, str);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_AP_DESCRIPTOR);
}

APBool APAssembleAPIPAddr(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleAPIPAddr()");
	u32 ip = APGetAPIPAddr();

	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 4, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleAPIPAddr()"););
	APDebugLog(5, "Assemble AP IPAddr: %u.%u.%u.%u", (u8)(ip >> 24), (u8)(ip >> 16),\
	  (u8)(ip >> 8),  (u8)(ip >> 0));
	APProtocolStore32(msgPtr, ip);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_AP_IP_ADDR);
}

APBool APAssembleAPMACAddr(APProtocolMessage *msgPtr) 
{
	int i;
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleAPMACAddr()");
	u8 *mac = APGetAPMACAddr();

	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 6, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleAPMACAddr()"););
	APDebugLog(5, "Assemble AP MACAddr: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1],\
	 mac[2], mac[3], mac[4], mac[5]);
	for(i = 0; i < 6; i++) {
		APProtocolStore8(msgPtr, mac[i]);
	}

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_AP_MAC_ADDR);
}

APBool APAssembleDiscoveryType(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleDiscoveryType()");
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleDiscoveryType()"););
	APDebugLog(5, "Assemble Discovery Type: 0x%02x", APGetDiscoveryType());
	APProtocolStore8(msgPtr, APGetDiscoveryType());

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_DISCOVERY_TYPE);
}

APBool APParseResultCode(APProtocolMessage *msgPtr, int len, u16 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APParseResultCode()");
	
	*valPtr = APProtocolRetrieve16(msgPtr);
	APDebugLog(5, "Parse Result Code: 0x%04x", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		APErrorLog("Message Element Malformed");
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseResultCode()");
	}
	return AP_TRUE;
}

APBool APParseReasonCode(APProtocolMessage *msgPtr, int len, u16 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APParseReasonCode()");
	
	*valPtr = APProtocolRetrieve16(msgPtr);
	APDebugLog(5, "Parse Reason Code: 0x%04x", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		APErrorLog("Message Element Malformed");
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseReasonCode()");
	}
	return AP_TRUE;
}

APBool APParseAssignedAPID(APProtocolMessage *msgPtr, int len, u16 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APParseAssignedAPID()");
	
	*valPtr = APProtocolRetrieve16(msgPtr);
	APDebugLog(5, "Parse Assigned APID: %u", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		APErrorLog("Message Element Malformed");
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseAssignedAPID()");
	}
	return AP_TRUE;
}

APBool APParseRegisteredService(APProtocolMessage *msgPtr, int len, u8 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APParseRegisteredService()");
	
	*valPtr = APProtocolRetrieve8(msgPtr);
	APDebugLog(5, "Parse Registered Service: 0x%02x", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		APErrorLog("Message Element Malformed");
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseRegisteredService()");
	}
	return AP_TRUE;
}

APBool APAssembleSSID(APProtocolMessage *msgPtr) 
{
	char* str;
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleSSID()");
	str = APGetSSID();
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, strlen(str), return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleSSID()"););
	APDebugLog(5, "Assemble SSID: %s", str);
	APProtocolStoreStr(msgPtr, str);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_SSID);
}

APBool APAssembleChannel(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleChannel()");
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleChannel()"););
	APDebugLog(5, "Assemble Channel: %u", APGetChannel());
	APProtocolStore8(msgPtr, APGetChannel());

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_CHANNEL);
}

APBool APAssembleHardwareMode(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleHardwareMode()");
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleHardwareMode()"););
	APDebugLog(5, "Assemble Hardware Mode: %u", APGetHardwareMode());
	APProtocolStore8(msgPtr, APGetHardwareMode());

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_HARDWARE_MODE);
}

APBool APAssembleSuppressSSID(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleSuppressSSID()");
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleSuppressSSID()"););
	APDebugLog(5, "Assemble Suppress SSID: %u", APGetSuppressSSID());
	APProtocolStore8(msgPtr, APGetSuppressSSID());

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_SUPPRESS_SSID);
}

APBool APAssembleSecurityOption(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleSecurityOption()");
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleSecurityOption()"););
	APDebugLog(5, "Assemble Security Option: %u", APGetSecurityOption());
	APProtocolStore8(msgPtr, APGetSecurityOption());

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_SECURITY_OPTION);
}