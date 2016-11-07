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

	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	AP_INIT_PROTOCOL_MESSAGE(completeMsg, ELEMENT_HEADER_LEN+(msgPtr->offset), return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););

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
		return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);

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
		return AP_FALSE;
	}

	AP_INIT_PROTOCOL_MESSAGE(completeMsg, controlHdr.offset + msgElemsLen, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
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
	if(controlHdrPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);

	AP_INIT_PROTOCOL_MESSAGE(*controlHdrPtr, HEADER_LEN, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););

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
	if(msgPtr == NULL|| valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);

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


APBool APParseControllerName(APProtocolMessage *msgPtr, int len, char **valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	*valPtr = APProtocolRetrieveStr(msgPtr, len);
	if(valPtr == NULL) return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL);
	APDebugLog(5, "Controller Name: %s", *valPtr);
	return ((msgPtr->offset) - oldOffset) == len ? AP_TRUE :\
		APErrorRaise(AP_ERROR_INVALID_FORMAT, "Message Element Malformed");
}

APBool APParseControllerDescriptor(APProtocolMessage *msgPtr, int len, char **valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	*valPtr = APProtocolRetrieveStr(msgPtr, len);
	if(valPtr == NULL) return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL);
	APDebugLog(5, "Controller Descriptor: %s", *valPtr);
	return ((msgPtr->offset) - oldOffset) == len ? AP_TRUE :\
		APErrorRaise(AP_ERROR_INVALID_FORMAT, "Message Element Malformed");
}

APBool APParseControllerIPAddr(APProtocolMessage *msgPtr, int len, u32 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	*valPtr = APProtocolRetrieve32(msgPtr);
	APDebugLog(5, "Controller IP:  %u.%u.%u.%u", (u8)(*valPtr >> 24), (u8)(*valPtr >> 16),\
	  (u8)(*valPtr >> 8),  (u8)(*valPtr >> 0));
	return ((msgPtr->offset) - oldOffset) == len ? AP_TRUE :\
		APErrorRaise(AP_ERROR_INVALID_FORMAT, "Message Element Malformed");
}

APBool APParseControllerMACAddr(APProtocolMessage *msgPtr, int len, u8 *valPtr) 
{	
	int i;
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	for(i = 0; i < 6; i++) {
		valPtr[i] = APProtocolRetrieve8(msgPtr);
	}
	APDebugLog(5, "Controller MAC:  %02x:%02x:%02x:%02x:%02x:%02x", valPtr[0], valPtr[1],\
	 valPtr[2], valPtr[3], valPtr[4], valPtr[5]);
	return ((msgPtr->offset) - oldOffset) == len ? AP_TRUE :\
		APErrorRaise(AP_ERROR_INVALID_FORMAT, "Message Element Malformed");
}

APBool APAssembleRegisteredService(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
	// APDebugLog(3, "Registered Service: %d", APGetRegisteredService());
	APProtocolStore8(msgPtr, APGetRegisteredService());

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_REGISTERED_SERVICE);
}

APBool APAssembleAPName(APProtocolMessage *msgPtr) 
{
	char* str;
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	str = APGetAPName();
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, strlen(str), return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
	// APDebugLog(3, "AP Name: %s", str);
	APProtocolStoreStr(msgPtr, str);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_AP_NAME);
}

APBool APAssembleAPDescriptor(APProtocolMessage *msgPtr) 
{
	char* str;
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	str = APGetAPDescriptor();
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, strlen(str), return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
	// APDebugLog(3, "AP Descriptor: %s", str);
	APProtocolStoreStr(msgPtr, str);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_AP_DESCRIPTOR);
}

APBool APAssembleAPIPAddr(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 4, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
	APProtocolStore32(msgPtr, APGetAPIPAddr());

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_AP_IP_ADDR);
}

APBool APAssembleAPMACAddr(APProtocolMessage *msgPtr) 
{
	int i;
	u8 *mac = APGetAPMACAddr();
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 6, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
	for(i = 0; i < 6; i++) {
		APProtocolStore8(msgPtr, mac[i]);
	}

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_AP_MAC_ADDR);
}

APBool APAssembleDiscoveryType(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
	// APDebugLog(3, "Discovery Type: %d", APGetDiscoveryType());
	APProtocolStore8(msgPtr, APGetDiscoveryType());

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_DISCOVERY_TYPE);
}

APBool APParseResultCode(APProtocolMessage *msgPtr, int len, u16 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	*valPtr = APProtocolRetrieve16(msgPtr);
	APDebugLog(5, "Result Code:  %u", *valPtr);
	return ((msgPtr->offset) - oldOffset) == len ? AP_TRUE :\
		APErrorRaise(AP_ERROR_INVALID_FORMAT, "Message Element Malformed");
}

APBool APParseReasonCode(APProtocolMessage *msgPtr, int len, u16 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	*valPtr = APProtocolRetrieve16(msgPtr);
	APDebugLog(5, "Reason Code:  %u", *valPtr);
	return ((msgPtr->offset) - oldOffset) == len ? AP_TRUE :\
		APErrorRaise(AP_ERROR_INVALID_FORMAT, "Message Element Malformed");
}

APBool APParseAssignedAPID(APProtocolMessage *msgPtr, int len, u16 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	*valPtr = APProtocolRetrieve16(msgPtr);
	APDebugLog(5, "Assigned APID:  %d", *valPtr);
	return ((msgPtr->offset) - oldOffset) == len ? AP_TRUE :\
		APErrorRaise(AP_ERROR_INVALID_FORMAT, "Message Element Malformed");
}

APBool APAssembleSSID(APProtocolMessage *msgPtr) 
{
	char* str;
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	str = APGetSSID();
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, strlen(str), return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
	// APDebugLog(3, "AP Name: %s", str);
	APProtocolStoreStr(msgPtr, str);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_SSID);
}

APBool APAssembleChannel(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
	APProtocolStore8(msgPtr, APGetChannel());

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_CHANNEL);
}

APBool APAssembleHardwareMode(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
	APProtocolStore8(msgPtr, APGetHardwareMode());

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_HARDWARE_MODE);
}

APBool APAssembleSuppressSSID(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
	APProtocolStore8(msgPtr, APGetSuppressSSID());

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_SUPPRESS_SSID);
}

APBool APAssembleSecuritySetting(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
	APProtocolStore8(msgPtr, APGetSecuritySetting());

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_SECURITY_SETTING);
}