#include "protocol.h"

/**
 * store a 8-bit value into a initialized APProtocolMessage
 * @param msgPtr [a initialized APProtocolMessage and there is space left]
 * @param val    [value you want to store]
 */
void APProtocolStore8(APProtocolMessage *msgPtr, u8 val)
{
	copy_memory(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 1);
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
	copy_memory(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 2);
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
	copy_memory(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 4);
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
	copy_memory(&((msgPtr->msg)[(msgPtr->offset)]), str, len);
	(msgPtr->offset) += len;
}

/**
 * store content in another APProtocolMessage into a initialized APProtocolMessage
 * @param msgPtr        [a initialized APProtocolMessage and there is space left]
 * @param msgToStorePtr [another APProtocolMessage you want to use]
 */
void APProtocolStoreMessage(APProtocolMessage *msgPtr, APProtocolMessage *msgToStorePtr)
{
	copy_memory(&((msgPtr->msg)[(msgPtr->offset)]), msgToStorePtr->msg, msgToStorePtr->offset);
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
	copy_memory(&((msgPtr->msg)[(msgPtr->offset)]), bytes, len);
	(msgPtr->offset) += len;
}

/**
 * store reserved value, that is 0, into a initialized APProtocolMessage
 * @param msgPtr      [a initialized APProtocolMessage and there is space left]
 * @param reservedLen [size of reserved space]
 */
void APProtocolStoreReserved(APProtocolMessage *msgPtr, int reservedLen)
{
	zero_memory(&((msgPtr->msg)[(msgPtr->offset)]), reservedLen);
	(msgPtr->offset) += reservedLen;
}

/**
 * assemble a formatted msg element by the element content and type
 * @param  msgPtr [the element content, which is preserved in a unformatted APProtocolMessage. after this action, it will transform a formatted msg element]
 * @param  type   [the type of msg elem]
 * @return        [whether the operation is success or not]
 */
bool APAssembleMsgElem(APProtocolMessage *msgPtr, u16 type)
{
	APProtocolMessage completeMsg;

	if(msgPtr == NULL) return false;
	AP_INIT_PROTOCOL_MESSAGE(completeMsg, ELEMENT_HEADER_LEN+(msgPtr->offset), return false;);

	APProtocolStore16(&completeMsg, type);
	APProtocolStore16(&completeMsg, msgPtr->offset);
	APProtocolStoreMessage(&completeMsg, msgPtr);

	AP_FREE_PROTOCOL_MESSAGE(*msgPtr);

	msgPtr->msg = completeMsg.msg;
	msgPtr->offset = completeMsg.offset;
	msgPtr->type = type;

	return true;
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
bool APAssembleControlMessage(APProtocolMessage *msgPtr, u16 apid, u32 seqNum,
						 u16 msgType, APProtocolMessage *msgElems, int msgElemNum)
{
	APProtocolMessage controlHdr, completeMsg;
	int msgElemsLen = 0, i;
	APHeaderVal controlHdrVal;
	AP_INIT_PROTOCOL(controlHdr);

	if(msgPtr == NULL || (msgElems == NULL && msgElemNum > 0))
		return false;

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
		return false;
	}

	AP_INIT_PROTOCOL_MESSAGE(completeMsg, controlHdr.offset + msgElemsLen, return false;);
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

	return true;
}

/**
 * assemble a msg header
 * @param  controlHdrPtr [a APAssembleMessage type value, which will output a formatted msg header]
 * @param  valPtr        [header info]
 * @return               [whether the operation is success or not]
 */
bool APAssembleControlHeader(APProtocolMessage *controlHdrPtr, APHeaderVal *valPtr)
{
	if(controlHdrPtr == NULL || valPtr == NULL) return false;

	AP_INIT_PROTOCOL_MESSAGE(*controlHdrPtr, HEADER_LEN, return false;);

	APProtocolStore8(controlHdrPtr, valPtr->version);
	APProtocolStore8(controlHdrPtr, valPtr->type);
	APProtocolStore16(controlHdrPtr, valPtr->apid);
	APProtocolStore32(controlHdrPtr, valPtr->seqNum);
	APProtocolStore16(controlHdrPtr, valPtr->msgType);
	APProtocolStore16(controlHdrPtr, valPtr->msgLen);
	APProtocolStoreReserved(controlHdrPtr, 4);

	return true;
}

/**
 * get a 8-bit value from APProtocolMessage
 * @param  msgPtr [a APProtocolMessage that include value]
 * @return        [u8 value]
 */
u8 APProtocolRetrieve8(APProtocolMessage *msgPtr) 
{
	u8 val;

	copy_memory(&val, &((msgPtr->msg)[(msgPtr->offset)]), 1);
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

	copy_memory(&val, &((msgPtr->msg)[(msgPtr->offset)]), 2);
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

	copy_memory(&val, &((msgPtr->msg)[(msgPtr->offset)]), 4);
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

	create_object(str, (len+1), return NULL;);

	copy_memory(str, &((msgPtr->msg)[(msgPtr->offset)]), len);
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

	create_object(bytes, len, return NULL;);

	copy_memory(bytes, &((msgPtr->msg)[(msgPtr->offset)]), len);
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


bool APParseControlHeader(APProtocolMessage *msgPtr, APHeaderVal *valPtr) 
{	
	if(msgPtr == NULL|| valPtr == NULL) return false;

	valPtr->version = APProtocolRetrieve8(msgPtr);
	valPtr->type = APProtocolRetrieve8(msgPtr);
	valPtr->apid = APProtocolRetrieve16(msgPtr);
	valPtr->seqNum = APProtocolRetrieve32(msgPtr);
	valPtr->msgType = APProtocolRetrieve16(msgPtr);
	valPtr->msgLen = APProtocolRetrieve16(msgPtr);
	APProtocolRetrieveReserved(msgPtr, 4);
	
	return true;
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

bool APParseControllerName(APProtocolMessage *msgPtr, int len, char **valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieveStr(msgPtr, len);
	if(valPtr == NULL) return false;
	log_d(5, "Parse Controller Name: %s", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseControllerDescriptor(APProtocolMessage *msgPtr, int len, char **valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieveStr(msgPtr, len);
	if(valPtr == NULL) return false;
	log_d(5, "Parse Controller Descriptor: %s", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseControllerIPAddr(APProtocolMessage *msgPtr, int len, u32 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieve32(msgPtr);
	log_d(5, "Parse Controller IP: %u.%u.%u.%u", (u8)(*valPtr >> 24), (u8)(*valPtr >> 16),\
	  (u8)(*valPtr >> 8),  (u8)(*valPtr >> 0));

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseControllerMACAddr(APProtocolMessage *msgPtr, int len, u8 *valPtr) 
{	
	int i;
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	for(i = 0; i < 6; i++) {
		valPtr[i] = APProtocolRetrieve8(msgPtr);
	}
	log_d(5, "Parse Controller MAC: %02x:%02x:%02x:%02x:%02x:%02x", valPtr[0], valPtr[1],\
	 valPtr[2], valPtr[3], valPtr[4], valPtr[5]);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseControllerNextSeq(APProtocolMessage *msgPtr, int len, u32 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieve32(msgPtr);
	log_d(5, "Parse Controller Next Seq: %d", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APAssembleRegisteredService(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return false;
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return false;);
	log_d(5, "Assemble Registered Service: 0x%02x", ap_register_service);
	APProtocolStore8(msgPtr, ap_register_service);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_REGISTERED_SERVICE);
}

bool APAssembleAPName(APProtocolMessage *msgPtr) 
{
	char* str;
	if(msgPtr == NULL) return false;
	str = ap_name;
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, strlen(str), return false;);
	log_d(5, "Assemble AP Name: %s", str);
	APProtocolStoreStr(msgPtr, str);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_AP_NAME);
}

bool APAssembleAPDescriptor(APProtocolMessage *msgPtr) 
{
	char* str;
	if(msgPtr == NULL) return false;
	str = ap_desc;
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, strlen(str), return false;);
	log_d(5, "Assemble AP Descriptor: %s", str);
	APProtocolStoreStr(msgPtr, str);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_ap_descCRIPTOR);
}

bool APAssembleAPIPAddr(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return false;
	u32 ip = ap_ip;

	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 4, return false;);
	log_d(5, "Assemble AP IPAddr: %u.%u.%u.%u", (u8)(ip >> 24), (u8)(ip >> 16),\
	  (u8)(ip >> 8),  (u8)(ip >> 0));
	APProtocolStore32(msgPtr, ip);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_AP_IP_ADDR);
}

bool APAssembleAPMACAddr(APProtocolMessage *msgPtr) 
{
	int i;
	if(msgPtr == NULL) return false;
	u8 *mac = ap_mac;

	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 6, return false;);
	log_d(5, "Assemble AP MACAddr: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1],\
	 mac[2], mac[3], mac[4], mac[5]);
	for(i = 0; i < 6; i++) {
		APProtocolStore8(msgPtr, mac[i]);
	}

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_AP_MAC_ADDR);
}

bool APAssembleDiscoveryType(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return false;
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return false;);
	log_d(5, "Assemble Discovery Type: 0x%02x", ap_discovery_type);
	APProtocolStore8(msgPtr, ap_discovery_type);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_DISCOVERY_TYPE);
}

bool APParseResultCode(APProtocolMessage *msgPtr, int len, u16 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieve16(msgPtr);
	log_d(5, "Parse Result Code: 0x%04x", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseReasonCode(APProtocolMessage *msgPtr, int len, u16 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieve16(msgPtr);
	log_d(5, "Parse Reason Code: 0x%04x", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseAssignedAPID(APProtocolMessage *msgPtr, int len, u16 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieve16(msgPtr);
	log_d(5, "Parse Assigned APID: %u", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseRegisteredService(APProtocolMessage *msgPtr, int len, u8 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieve8(msgPtr);
	log_d(5, "Parse Registered Service: 0x%02x", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APAssembleSSID(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return false;
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, strlen(wlconf->conf->ssid), return false;);
	log_d(5, "Assemble SSID: %s", wlconf->conf->ssid);
	APProtocolStoreStr(msgPtr, wlconf->conf->ssid);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_SSID);
}

bool APAssembleChannel(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return false;
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return false;);
	log_d(5, "Assemble Channel: %u", wlconf->conf->channel);
	APProtocolStore8(msgPtr, wlconf->conf->channel);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_CHANNEL);
}

bool APAssembleHardwareMode(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return false;
	
	u8 hwMode = 0xFF;
	if(!strcmp(wlconf->conf->hwmode, ONLY_A)) hwMode = HWMODE_A;
	else if(!strcmp(wlconf->conf->hwmode, ONLY_B)) hwMode = HWMODE_B;
	else if(!strcmp(wlconf->conf->hwmode, ONLY_G)) hwMode = HWMODE_G;
	else if(!strcmp(wlconf->conf->hwmode, ONLY_N)) hwMode = HWMODE_N;
	if(hwMode == 0xFF) return false;

	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return false;);
	log_d(5, "Assemble Hardware Mode: %u", hwMode);
	APProtocolStore8(msgPtr, hwMode);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_HARDWARE_MODE);
}

bool APAssembleSuppressSSID(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return false;
	
	u8 suppress = wlconf->conf->hidden ? 1 : 0;

	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return false;);
	log_d(5, "Assemble Suppress SSID: %u", suppress);
	APProtocolStore8(msgPtr, suppress);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_SUPPRESS_SSID);
}

bool APAssembleSecurityOption(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return false;
	
	u8 security = 0xFF;
	if(wlconf->conf->encryption == NULL || strlen(wlconf->conf->encryption) == 0) security = SECURITY_OPEN;
	else if(!strcmp(wlconf->conf->encryption, NO_ENCRYPTION)) security = SECURITY_OPEN;
	else if(!strcmp(wlconf->conf->encryption, WPA_WPA2_MIXED)) security = SECURITY_WPA_WPA2_MIXED;
	else if(!strcmp(wlconf->conf->encryption, WPA_PSK)) security = SECURITY_WPA;
	else if(!strcmp(wlconf->conf->encryption, WPA2_PSK)) security = SECURITY_WPA2;
	if(security == 0xFF) return false;

	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return false;);
	log_d(5, "Assemble Security Option: %u", security);
	APProtocolStore8(msgPtr, security);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_SECURITY_OPTION);
}

bool APAssembleMACFilterMode(APProtocolMessage *msgPtr)
{
	if(msgPtr == NULL) return false;
	
	u8 mode = 0xFF;
	if(wlconf->conf->macfilter == NULL || strlen(wlconf->conf->macfilter) == 0) mode = FILTER_NONE;
	else if(!strcmp(wlconf->conf->macfilter, MAC_FILTER_NONE)) mode = FILTER_NONE;
	else if(!strcmp(wlconf->conf->macfilter, MAC_FILTER_ALLOW)) mode = FILTER_ACCEPT_ONLY;
	else if(!strcmp(wlconf->conf->macfilter, MAC_FILTER_DENY)) mode = FILTER_DENY;
	if(mode == 0xFF) return false;

	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return false;);
	log_d(5, "Assemble MAC Filter Mode: %u", mode);
	APProtocolStore8(msgPtr, mode);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_MACFILTER_MODE);
}

bool APAssembleMACFilterList(APProtocolMessage *msgPtr)
{
	if(msgPtr == NULL) return false;
	
	int list_num = wlconf->conf->macfilter_list->listsize;
	if(list_num == 0) {
		log_d(5, "Assemble MAC Filter List Size: 0");
		return APAssembleMsgElem(msgPtr, MSGELEMTYPE_MACFILTER_LIST);
	} 

	log_d(5, "Assemble MAC Filter List Size: %d", list_num);
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, list_num * 6, return false;);

	struct maclist_node *node;
	mlist_foreach_element(wlconf->conf->macfilter_list, node)
	{
		int i;
		int mac[6];
		mac_to_hex(node->macaddr, mac);
		for(i = 0; i < 6; i++) {
			APProtocolStore8(msgPtr, mac[i]);
		}
		log_d(5, "Assemble MAC Filter List: %s", node->macaddr);
	}

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_MACFILTER_LIST);
}

bool APAssembleTxPower(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return false;
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, 1, return false;);
	log_d(5, "Assemble Tx Power: %u", wlconf->conf->txpower);
	APProtocolStore8(msgPtr, wlconf->conf->txpower);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_TX_POWER);
}

bool APAssembleWPAPassword(APProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return false;
	
	AP_INIT_PROTOCOL_MESSAGE(*msgPtr, strlen(wlconf->conf->key), return false;);
	log_d(5, "Assemble WPA Password: %s", wlconf->conf->key);
	APProtocolStoreStr(msgPtr, wlconf->conf->key);

	return APAssembleMsgElem(msgPtr, MSGELEMTYPE_WPA_PWD);
}

bool APParseDesiredConfList(APProtocolMessage *msgPtr, int len, u8 **valPtr)
{
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieveRawBytes(msgPtr, len);

	// while(pos < len) {
	// 	u16 type = APProtocolRetrieve16(msgPtr);
	// 	log_d(5, "Parse Desired Conf List: 0x%04x", type);
	// 	copy_memory(valPtr + pos, &type, 2);
	// 	pos += 2;
	// }
	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
}

bool APParseSSID(APProtocolMessage *msgPtr, int len, char **valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieveStr(msgPtr, len);
	if(valPtr == NULL) return false;
	log_d(5, "Parse SSID: %s", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseChannel(APProtocolMessage *msgPtr, int len, u8 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieve8(msgPtr);
	log_d(5, "Parse Channel: %u", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseHardwareMode(APProtocolMessage *msgPtr, int len, u8 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieve8(msgPtr);
	log_d(5, "Parse Hardware Mode: %u", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseSuppressSSID(APProtocolMessage *msgPtr, int len, u8 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieve8(msgPtr);
	log_d(5, "Parse Suppress SSID: %u", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseSecurityOption(APProtocolMessage *msgPtr, int len, u8 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieve8(msgPtr);
	log_d(5, "Parse Security Option: %u", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseMACFilterMode(APProtocolMessage *msgPtr, int len, u8 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieve8(msgPtr);
	log_d(5, "Parse MAC Filter Mode: %u", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseTxPower(APProtocolMessage *msgPtr, int len, u8 *valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieve8(msgPtr);
	log_d(5, "Parse Tx Power: %u", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseWPAPassword(APProtocolMessage *msgPtr, int len, char **valPtr) 
{	
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieveStr(msgPtr, len);
	if(valPtr == NULL) return false;
	log_d(5, "Parse WPA Password: %s", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseMACList(APProtocolMessage *msgPtr, int len, char ***valPtr)
{
	int oldOffset = msgPtr->offset;
	u8 mac[6];
	int pos = 0, cnt = 0;
	if(msgPtr == NULL || valPtr == NULL) return false;

	while(pos < len) 
	{
		mac[pos % 6] = APProtocolRetrieve8(msgPtr);
		if(pos % 6 == 5) 
		{
			char *str;
			create_string(str, 18, return false;);
			sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x\0", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			(*valPtr)[cnt++] = str;
			log_d(5, "Parse MAC List: %02x:%02x:%02x:%02x:%02x:%02x\0", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
		pos++;
	}
	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool APParseSystemCommand(APProtocolMessage *msgPtr, int len, u8 *valPtr)
{
	int oldOffset = msgPtr->offset;
	if(msgPtr == NULL || valPtr == NULL) return false;
	
	*valPtr = APProtocolRetrieve8(msgPtr);
	log_d(5, "Parse System Command: %u", *valPtr);

	if((msgPtr->offset - oldOffset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}