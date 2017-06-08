#include "protocol.h"

/**
 * store a 8-bit value into a initialized protocol_msg
 * @param msg_p [a initialized protocol_msg and there is space left]
 * @param val    [value you want to store]
 */
void store_8(protocol_msg *msg_p, u8 val)
{
	copy_memory(&((msg_p->msg)[(msg_p->offset)]), &(val), 1);
	(msg_p->offset) += 1;
}

/**
 * store a 16-bit value into a initialized protocol_msg
 * @param msg_p [a initialized protocol_msg and there is space left]
 * @param val    [value you want to store]
 */
void store_16(protocol_msg *msg_p, u16 val)
{
	val = htons(val);
	copy_memory(&((msg_p->msg)[(msg_p->offset)]), &(val), 2);
	(msg_p->offset) += 2;
}

/**
 * store a 32-bit value into a initialized protocol_msg
 * @param msg_p [a initialized protocol_msg and there is space left]
 * @param val    [value you want to store]
 */
void store_32(protocol_msg *msg_p, u32 val)
{
	val = htonl(val);
	copy_memory(&((msg_p->msg)[(msg_p->offset)]), &(val), 4);
	(msg_p->offset) += 4;
}

/**
 * store a string into a initialized protocol_msg
 * @param msg_p [a initialized protocol_msg and there is space left]
 * @param str    [value you want to store, a standard string must end with '\0']
 */
void store_str(protocol_msg *msg_p, char *str)
{
	int len = strlen(str);
	copy_memory(&((msg_p->msg)[(msg_p->offset)]), str, len);
	(msg_p->offset) += len;
}

/**
 * store content in another protocol_msg into a initialized protocol_msg
 * @param msg_p        [a initialized protocol_msg and there is space left]
 * @param from_p [another protocol_msg you want to use]
 */
void store_msg(protocol_msg *msg_p, protocol_msg *from_p)
{
	copy_memory(&((msg_p->msg)[(msg_p->offset)]), from_p->msg, from_p->offset);
	(msg_p->offset) += from_p->offset;
}

/**
 * store raw bytes into a initialized protocol_msg
 * @param msg_p [a initialized protocol_msg and there is space left]
 * @param bytes  [value you want to store]
 * @param len    [size of raw bytes]
 */
void store_raw(protocol_msg *msg_p, u8 *bytes, int len)
{
	copy_memory(&((msg_p->msg)[(msg_p->offset)]), bytes, len);
	(msg_p->offset) += len;
}

/**
 * store reserved value, that is 0, into a initialized protocol_msg
 * @param msg_p      [a initialized protocol_msg and there is space left]
 * @param reserved_len [size of reserved space]
 */
void store_reserved(protocol_msg *msg_p, int reserved_len)
{
	zero_memory(&((msg_p->msg)[(msg_p->offset)]), reserved_len);
	(msg_p->offset) += reserved_len;
}

/**
 * assemble a formatted msg element by the element content and type
 * @param  msg_p [the element content, which is preserved in a unformatted protocol_msg. after this action, it will transform a formatted msg element]
 * @param  type   [the type of msg elem]
 * @return        [whether the operation is success or not]
 */
bool assemble_msgelem(protocol_msg *msg_p, u16 type)
{
	protocol_msg complete_msg;

	if(msg_p == NULL) return false;
	init_protocol_msg_size(complete_msg, ELEMENT_HEADER_LEN+(msg_p->offset), return false;);

	store_16(&complete_msg, type);
	store_16(&complete_msg, msg_p->offset);
	store_msg(&complete_msg, msg_p);

	free_protocol_msg(*msg_p);

	msg_p->msg = complete_msg.msg;
	msg_p->offset = complete_msg.offset;
	msg_p->type = type;

	return true;
}

/**
 * assemble a formatted msg that can be send
 * @param  msg_p     [a APAssembleMessage type value, which will output a formatted msg]
 * @param  apid       [apid that will be used in message header]
 * @param  seq_num     [seq num that will be used in message header]
 * @param  msg_type    [the type of msg]
 * @param  msgelems   [a array of msgelems, which will be used in msg]
 * @param  msgElemNum [count of msgelems]
 * @return            [whether the operation is success or not]
 */
bool assemble_msg(protocol_msg *msg_p, u16 apid, u32 seq_num,
						 u16 msg_type, protocol_msg *msgelems, int msgElemNum)
{
	protocol_msg controlHdr, complete_msg;
	int msgElemsLen = 0, i;
	header_val controlHdrVal;
	init_protocol_msg(controlHdr);

	if(msg_p == NULL || (msgelems == NULL && msgElemNum > 0))
		return false;

	for(i = 0; i < msgElemNum; i++) msgElemsLen += msgelems[i].offset;

	controlHdrVal.version = CURRENT_VERSION;
	controlHdrVal.type = TYPE_CONTROL;
	controlHdrVal.apid = apid;
	controlHdrVal.seq_num = seq_num;
	controlHdrVal.msg_type = msg_type;
	controlHdrVal.msg_len = HEADER_LEN + msgElemsLen;

	if(!(assemble_header(&controlHdr, &controlHdrVal))) {
		free_protocol_msg(controlHdr);
		free_arr_and_protocol_msg(msgelems, msgElemNum);
		return false;
	}

	init_protocol_msg_size(complete_msg, controlHdr.offset + msgElemsLen, return false;);
	store_msg(&complete_msg, &controlHdr);
	free_protocol_msg(controlHdr);

	for(i = 0; i < msgElemNum; i++) {
		store_msg(&complete_msg, &(msgelems[i]));
	}
	free_arr_and_protocol_msg(msgelems, msgElemNum);

	free_protocol_msg(*msg_p);
	msg_p->msg = complete_msg.msg;
	msg_p->offset = complete_msg.offset;
	msg_p->type = msg_type;

	return true;
}

/**
 * assemble a msg header
 * @param  controlHdrPtr [a APAssembleMessage type value, which will output a formatted msg header]
 * @param  val_p        [header info]
 * @return               [whether the operation is success or not]
 */
bool assemble_header(protocol_msg *controlHdrPtr, header_val *val_p)
{
	if(controlHdrPtr == NULL || val_p == NULL) return false;

	init_protocol_msg_size(*controlHdrPtr, HEADER_LEN, return false;);

	store_8(controlHdrPtr, val_p->version);
	store_8(controlHdrPtr, val_p->type);
	store_16(controlHdrPtr, val_p->apid);
	store_32(controlHdrPtr, val_p->seq_num);
	store_16(controlHdrPtr, val_p->msg_type);
	store_16(controlHdrPtr, val_p->msg_len);
	store_reserved(controlHdrPtr, 4);

	return true;
}

/**
 * get a 8-bit value from protocol_msg
 * @param  msg_p [a protocol_msg that include value]
 * @return        [u8 value]
 */
u8 retrieve_8(protocol_msg *msg_p) 
{
	u8 val;

	copy_memory(&val, &((msg_p->msg)[(msg_p->offset)]), 1);
	(msg_p->offset) += 1;

	return val;
}

/**
 * get a 16-bit value from protocol_msg
 * @param  msg_p [a protocol_msg that include value]
 * @return        [u16 value]
 */
u16 retrieve_16(protocol_msg *msg_p) 
{
	u16 val;

	copy_memory(&val, &((msg_p->msg)[(msg_p->offset)]), 2);
	(msg_p->offset) += 2;

	return ntohs(val);
}

/**
 * get a 32-bit value from protocol_msg
 * @param  msg_p [a protocol_msg that include value]
 * @return        [u32 value]
 */
u32 retrieve_32(protocol_msg *msg_p) 
{
	u32 val;

	copy_memory(&val, &((msg_p->msg)[(msg_p->offset)]), 4);
	(msg_p->offset) += 4;

	return ntohl(val);
}

/**
 * get a string from protocol_msg
 * @param  msg_p [a protocol_msg that include value]
 * @param  len    [length of string]
 * @return        [a standard string that end with '\0']
 */
char* retrieve_str(protocol_msg *msg_p, int len) 
{
	u8* str;

	create_object(str, (len+1), return NULL;);

	copy_memory(str, &((msg_p->msg)[(msg_p->offset)]), len);
	str[len] = '\0';
	(msg_p->offset) += len;

	return (char*)str;
}

/**
 * get raw bytes from protocol_msg
 * @param  msg_p [a protocol_msg that include value]
 * @param  len    [size of raw bytes]
 * @return        [an array that include raw bytes]
 */
u8* retrieve_raw(protocol_msg *msg_p, int len) 
{
	u8* bytes;

	create_object(bytes, len, return NULL;);

	copy_memory(bytes, &((msg_p->msg)[(msg_p->offset)]), len);
	(msg_p->offset) += len;

	return bytes;
}

/**
 * pass reserved value
 * @param msg_p      [a protocol_msg]
 * @param reserved_len [size of reserved space]
 */
void retrieve_reserved(protocol_msg *msg_p, int reserved_len) 
{
	(msg_p->offset) += reserved_len;
}


bool parse_header(protocol_msg *msg_p, header_val *val_p) 
{	
	if(msg_p == NULL|| val_p == NULL) return false;

	val_p->version = retrieve_8(msg_p);
	val_p->type = retrieve_8(msg_p);
	val_p->apid = retrieve_16(msg_p);
	val_p->seq_num = retrieve_32(msg_p);
	val_p->msg_type = retrieve_16(msg_p);
	val_p->msg_len = retrieve_16(msg_p);
	retrieve_reserved(msg_p, 4);
	
	return true;
}

void parse_msgelem(protocol_msg *msg_p, u16 *type, u16 *len)
{
	*type = retrieve_16(msg_p);
	*len = retrieve_16(msg_p);
}

void parse_unrecognized_msgelem(protocol_msg *msg_p, int len)
{
	msg_p->offset += len;
}

void parse_repeated_msgelem(protocol_msg *msg_p, int len) 
{
	msg_p->offset += len;
}

bool parse_controller_name(protocol_msg *msg_p, int len, char **val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_str(msg_p, len);
	if(val_p == NULL) return false;
	log_d(5, "Parse Controller Name: %s", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_controller_desc(protocol_msg *msg_p, int len, char **val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_str(msg_p, len);
	if(val_p == NULL) return false;
	log_d(5, "Parse Controller Descriptor: %s", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_controller_ip(protocol_msg *msg_p, int len, u32 *val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_32(msg_p);
	log_d(5, "Parse Controller IP: %u.%u.%u.%u", (u8)(*val_p >> 24), (u8)(*val_p >> 16),\
	  (u8)(*val_p >> 8),  (u8)(*val_p >> 0));

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_controller_mac(protocol_msg *msg_p, int len, u8 *val_p) 
{	
	int i;
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	for(i = 0; i < 6; i++) {
		val_p[i] = retrieve_8(msg_p);
	}
	log_d(5, "Parse Controller MAC: %02x:%02x:%02x:%02x:%02x:%02x", val_p[0], val_p[1],\
	 val_p[2], val_p[3], val_p[4], val_p[5]);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_controller_nextseq(protocol_msg *msg_p, int len, u32 *val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_32(msg_p);
	log_d(5, "Parse Controller Next Seq: %d", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool assemble_register_service(protocol_msg *msg_p) 
{
	if(msg_p == NULL) return false;
	
	init_protocol_msg_size(*msg_p, 1, return false;);
	log_d(5, "Assemble Registered Service: 0x%02x", ap_register_service);
	store_8(msg_p, ap_register_service);

	return assemble_msgelem(msg_p, MSGELEMTYPE_REGISTERED_SERVICE);
}

bool assemble_ap_name(protocol_msg *msg_p) 
{
	char* str;
	if(msg_p == NULL) return false;
	str = ap_name;
	
	init_protocol_msg_size(*msg_p, strlen(str), return false;);
	log_d(5, "Assemble AP Name: %s", str);
	store_str(msg_p, str);

	return assemble_msgelem(msg_p, MSGELEMTYPE_AP_NAME);
}

bool assemble_ap_desc(protocol_msg *msg_p) 
{
	char* str;
	if(msg_p == NULL) return false;
	str = ap_desc;
	
	init_protocol_msg_size(*msg_p, strlen(str), return false;);
	log_d(5, "Assemble AP Descriptor: %s", str);
	store_str(msg_p, str);

	return assemble_msgelem(msg_p, MSGELEMTYPE_AP_DESCRIPTOR);
}

bool assemble_ap_ip(protocol_msg *msg_p) 
{
	if(msg_p == NULL) return false;
	u32 ip = ap_ip;

	init_protocol_msg_size(*msg_p, 4, return false;);
	log_d(5, "Assemble AP ip_addr: %u.%u.%u.%u", (u8)(ip >> 24), (u8)(ip >> 16),\
	  (u8)(ip >> 8),  (u8)(ip >> 0));
	store_32(msg_p, ip);

	return assemble_msgelem(msg_p, MSGELEMTYPE_AP_IP_ADDR);
}

bool assemble_ap_mac(protocol_msg *msg_p) 
{
	int i;
	if(msg_p == NULL) return false;
	u8 *mac = ap_mac;

	init_protocol_msg_size(*msg_p, 6, return false;);
	log_d(5, "Assemble AP mac_addr: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1],\
	 mac[2], mac[3], mac[4], mac[5]);
	for(i = 0; i < 6; i++) {
		store_8(msg_p, mac[i]);
	}

	return assemble_msgelem(msg_p, MSGELEMTYPE_AP_MAC_ADDR);
}

bool assemble_ap_discovery_type(protocol_msg *msg_p) 
{
	if(msg_p == NULL) return false;
	
	init_protocol_msg_size(*msg_p, 1, return false;);
	log_d(5, "Assemble Discovery Type: 0x%02x", ap_discovery_type);
	store_8(msg_p, ap_discovery_type);

	return assemble_msgelem(msg_p, MSGELEMTYPE_DISCOVERY_TYPE);
}

bool parse_res_code(protocol_msg *msg_p, int len, u16 *val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_16(msg_p);
	log_d(5, "Parse Result Code: 0x%04x", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_reason_code(protocol_msg *msg_p, int len, u16 *val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_16(msg_p);
	log_d(5, "Parse Reason Code: 0x%04x", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_assigned_apid(protocol_msg *msg_p, int len, u16 *val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_16(msg_p);
	log_d(5, "Parse Assigned APID: %u", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_register_service(protocol_msg *msg_p, int len, u8 *val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_8(msg_p);
	log_d(5, "Parse Registered Service: 0x%02x", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool assemble_ssid(protocol_msg *msg_p) 
{
	if(msg_p == NULL) return false;
	
	init_protocol_msg_size(*msg_p, strlen(wlconf->conf->ssid), return false;);
	log_d(5, "Assemble SSID: %s", wlconf->conf->ssid);
	store_str(msg_p, wlconf->conf->ssid);

	return assemble_msgelem(msg_p, MSGELEMTYPE_SSID);
}

bool assemble_channel(protocol_msg *msg_p) 
{
	if(msg_p == NULL) return false;
	
	init_protocol_msg_size(*msg_p, 1, return false;);
	log_d(5, "Assemble Channel: %u", wlconf->conf->channel);
	store_8(msg_p, wlconf->conf->channel);

	return assemble_msgelem(msg_p, MSGELEMTYPE_CHANNEL);
}

bool assemble_hwmode(protocol_msg *msg_p) 
{
	if(msg_p == NULL) return false;
	
	u8 hwMode = 0xFF;
	if(!strcmp(wlconf->conf->hwmode, ONLY_A)) hwMode = HWMODE_A;
	else if(!strcmp(wlconf->conf->hwmode, ONLY_B)) hwMode = HWMODE_B;
	else if(!strcmp(wlconf->conf->hwmode, ONLY_G)) hwMode = HWMODE_G;
	else if(!strcmp(wlconf->conf->hwmode, ONLY_N)) hwMode = HWMODE_N;
	if(hwMode == 0xFF) return false;

	init_protocol_msg_size(*msg_p, 1, return false;);
	log_d(5, "Assemble Hardware Mode: %u", hwMode);
	store_8(msg_p, hwMode);

	return assemble_msgelem(msg_p, MSGELEMTYPE_HARDWARE_MODE);
}

bool assemble_hide_ssid(protocol_msg *msg_p) 
{
	if(msg_p == NULL) return false;
	
	u8 suppress = wlconf->conf->hidden ? 1 : 0;

	init_protocol_msg_size(*msg_p, 1, return false;);
	log_d(5, "Assemble Suppress SSID: %u", suppress);
	store_8(msg_p, suppress);

	return assemble_msgelem(msg_p, MSGELEMTYPE_SUPPRESS_SSID);
}

bool assemble_sec_opt(protocol_msg *msg_p) 
{
	if(msg_p == NULL) return false;
	
	u8 security = 0xFF;
	if(wlconf->conf->encryption == NULL || strlen(wlconf->conf->encryption) == 0) security = SECURITY_OPEN;
	else if(!strcmp(wlconf->conf->encryption, NO_ENCRYPTION)) security = SECURITY_OPEN;
	else if(!strcmp(wlconf->conf->encryption, WPA_WPA2_MIXED)) security = SECURITY_WPA_WPA2_MIXED;
	else if(!strcmp(wlconf->conf->encryption, WPA_PSK)) security = SECURITY_WPA;
	else if(!strcmp(wlconf->conf->encryption, WPA2_PSK)) security = SECURITY_WPA2;
	if(security == 0xFF) return false;

	init_protocol_msg_size(*msg_p, 1, return false;);
	log_d(5, "Assemble Security Option: %u", security);
	store_8(msg_p, security);

	return assemble_msgelem(msg_p, MSGELEMTYPE_SECURITY_OPTION);
}

bool assemble_macfilter_mode(protocol_msg *msg_p)
{
	if(msg_p == NULL) return false;
	
	u8 mode = 0xFF;
	if(wlconf->conf->macfilter == NULL || strlen(wlconf->conf->macfilter) == 0) mode = FILTER_NONE;
	else if(!strcmp(wlconf->conf->macfilter, MAC_FILTER_NONE)) mode = FILTER_NONE;
	else if(!strcmp(wlconf->conf->macfilter, MAC_FILTER_ALLOW)) mode = FILTER_ACCEPT_ONLY;
	else if(!strcmp(wlconf->conf->macfilter, MAC_FILTER_DENY)) mode = FILTER_DENY;
	if(mode == 0xFF) return false;

	init_protocol_msg_size(*msg_p, 1, return false;);
	log_d(5, "Assemble MAC Filter Mode: %u", mode);
	store_8(msg_p, mode);

	return assemble_msgelem(msg_p, MSGELEMTYPE_MACFILTER_MODE);
}

bool assemble_macfilter_list(protocol_msg *msg_p)
{
	if(msg_p == NULL) return false;
	
	int list_num = wlconf->conf->macfilter_list->listsize;
	if(list_num == 0) {
		log_d(5, "Assemble MAC Filter List Size: 0");
		return assemble_msgelem(msg_p, MSGELEMTYPE_MACFILTER_LIST);
	} 

	log_d(5, "Assemble MAC Filter List Size: %d", list_num);
	init_protocol_msg_size(*msg_p, list_num * 6, return false;);

	struct maclist_node *node;
	mlist_foreach_element(wlconf->conf->macfilter_list, node)
	{
		int i;
		int mac[6];
		mac_to_hex(node->mac_addr, mac);
		for(i = 0; i < 6; i++) {
			store_8(msg_p, mac[i]);
		}
		log_d(5, "Assemble MAC Filter List: %s", node->mac_addr);
	}

	return assemble_msgelem(msg_p, MSGELEMTYPE_MACFILTER_LIST);
}

bool assemble_tx_power(protocol_msg *msg_p) 
{
	if(msg_p == NULL) return false;
	
	init_protocol_msg_size(*msg_p, 1, return false;);
	log_d(5, "Assemble Tx Power: %u", wlconf->conf->txpower);
	store_8(msg_p, wlconf->conf->txpower);

	return assemble_msgelem(msg_p, MSGELEMTYPE_TX_POWER);
}

bool assemble_wpa_pwd(protocol_msg *msg_p) 
{
	if(msg_p == NULL) return false;
	
	init_protocol_msg_size(*msg_p, strlen(wlconf->conf->key), return false;);
	log_d(5, "Assemble WPA Password: %s", wlconf->conf->key);
	store_str(msg_p, wlconf->conf->key);

	return assemble_msgelem(msg_p, MSGELEMTYPE_WPA_PWD);
}

bool parse_desired_conf_list(protocol_msg *msg_p, int len, u8 **val_p)
{
	int old_offset = msg_p->offset;
	if(msg_p == NULL) return false;
	
	*val_p = retrieve_raw(msg_p, len);

	// while(pos < len) {
	// 	u16 type = retrieve_16(msg_p);
	// 	log_d(5, "Parse Desired Conf List: 0x%04x", type);
	// 	copy_memory(val_p + pos, &type, 2);
	// 	pos += 2;
	// }
	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
}

bool parse_ssid(protocol_msg *msg_p, int len, char **val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_str(msg_p, len);
	if(val_p == NULL) return false;
	log_d(5, "Parse SSID: %s", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_channel(protocol_msg *msg_p, int len, u8 *val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_8(msg_p);
	log_d(5, "Parse Channel: %u", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_hwmode(protocol_msg *msg_p, int len, u8 *val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_8(msg_p);
	log_d(5, "Parse Hardware Mode: %u", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_hide_ssid(protocol_msg *msg_p, int len, u8 *val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_8(msg_p);
	log_d(5, "Parse Suppress SSID: %u", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_sec_opt(protocol_msg *msg_p, int len, u8 *val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_8(msg_p);
	log_d(5, "Parse Security Option: %u", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_macfilter_mode(protocol_msg *msg_p, int len, u8 *val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_8(msg_p);
	log_d(5, "Parse MAC Filter Mode: %u", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_tx_power(protocol_msg *msg_p, int len, u8 *val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_8(msg_p);
	log_d(5, "Parse Tx Power: %u", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_wpa_pwd(protocol_msg *msg_p, int len, char **val_p) 
{	
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_str(msg_p, len);
	if(val_p == NULL) return false;
	log_d(5, "Parse WPA Password: %s", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_mac_list(protocol_msg *msg_p, int len, char ***val_p)
{
	int old_offset = msg_p->offset;
	u8 mac[6];
	int pos = 0, cnt = 0;
	if(msg_p == NULL || val_p == NULL) return false;

	while(pos < len) 
	{
		mac[pos % 6] = retrieve_8(msg_p);
		if(pos % 6 == 5) 
		{
			char *str;
			create_string(str, 18, return false;);
			sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x\0", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			(*val_p)[cnt++] = str;
			log_d(5, "Parse MAC List: %02x:%02x:%02x:%02x:%02x:%02x\0", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
		pos++;
	}
	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}

bool parse_sys_cmd(protocol_msg *msg_p, int len, u8 *val_p)
{
	int old_offset = msg_p->offset;
	if(msg_p == NULL || val_p == NULL) return false;
	
	*val_p = retrieve_8(msg_p);
	log_d(5, "Parse System Command: %u", *val_p);

	if((msg_p->offset - old_offset) != len) {
		log_e("Message Element Malformed");
		return false;
	}
	return true;
}