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

u8 APProtocolRetrieve8(APProtocolMessage *msgPtr) {
	u8 val;

	AP_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 1);
	(msgPtr->offset) += 1;

	return val;
}

u16 APProtocolRetrieve16(APProtocolMessage *msgPtr) {
	u16 val;

	AP_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 2);
	(msgPtr->offset) += 2;

	return ntohs(val);
}

u32 APProtocolRetrieve32(APProtocolMessage *msgPtr) {
	u32 val;

	AP_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 4);
	(msgPtr->offset) += 4;

	return ntohl(val);
}

char* APProtocolRetrieveStr(APProtocolMessage *msgPtr, int len) {
	u8* str;

	AP_CREATE_OBJECT_SIZE(str, (len+1));

	AP_COPY_MEMORY(str, &((msgPtr->msg)[(msgPtr->offset)]), len);
	str[len] = '\0';
	(msgPtr->offset) += len;

	return (char*)str;
}

u8* APProtocolRetrieveRawBytes(APProtocolMessage *msgPtr, int len) {
	u8* bytes;

	AP_CREATE_OBJECT_SIZE(bytes, len);

	AP_COPY_MEMORY(bytes, &((msgPtr->msg)[(msgPtr->offset)]), len);
	(msgPtr->offset) += len;

	return bytes;
}

APBool APAssembleMsgElemAPDescriptor(APProtocolMessage *msgPtr) {
	if(msgPtr == NULL) return AP_FALSE;
	
	char *descriptor = APGetAPDescriptor();
	AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, strlen(descriptor));
	
	APProtocolStoreStr(msgPtr, descriptor);
	return APAssembleMsgElem(msgPtr, MSGELETYPE_AP_DESCRIPTOR);
}

APBool APAssembleMsgElemAPName(APProtocolMessage *msgPtr) {
	if(msgPtr == NULL) return AP_FALSE;

	char *name = APGetAPName();
	AP_CREATE_PROTOCOL_MESSAGE(*msgPtr, strlen(name));

	APProtocolStoreStr(msgPtr, name);
	return APAssembleMsgElem(msgPtr, MSGELETYPE_AP_NAME);
}

APBool APAssembleMsgElemAPRadioInformation(APProtocolMessage *msgPtr) {
	if(msgPtr == NULL) return AP_FALSE;
	APProtocolStoreVoid(msgPtr);
	return APAssembleMsgElem(msgPtr, MSGELETYPE_AP_RADIO_INFORMATION);
}

