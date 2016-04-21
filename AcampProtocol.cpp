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

void APProtocolDestroyMsgElemData(void *f)
{
    AP_FREE_OBJECT(f);
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
    msgPtr->data_msgType = type;

    return AP_TRUE;
}
