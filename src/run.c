#include "common.h"
#include "error.h"
#include "log.h"
#include "ap.h"
#include "network.h"

#define MAX_EVENT_NUMBER 1024

struct uloop_fd fdSocket;
struct uloop_timeout tKeepAlive;
struct uloop_timeout tRetransmit;
protocol_msg retransmitMsg;
int retransmitInterval;
int retransmitCount;
protocol_msg cacheMsg;

static void APRretransmitHandler(struct uloop_timeout *t)
{
	retransmitCount++;
	if(retransmitCount >= 5) {
		log_i("There is no valid Response for %d times", retransmitCount);
		log_i("The connection to controller has been interrupted");
		uloop_end();
		return;
	}

	log_i("There is no valid Response, times = %d, retransmit request", retransmitCount);
	retransmitInterval *= 2;
	if(retransmitInterval > keepalive_interval / 2) {
		retransmitInterval = keepalive_interval / 2;
	}
	uloop_timeout_set(&tRetransmit, retransmitInterval * 1000);
	log_d(5, "Adjust the retransmit interval to %d sec and retransmit", retransmitInterval);
	if(!(send_udp(retransmitMsg))) {
		log_e("Failed to retransmit Request");
		uloop_end();
		return;
	}
}

bool APAssembleKeepAliveRequest(protocol_msg *messagesPtr)
{
	int k = -1;
	if(messagesPtr == NULL) return false;
	
	protocol_msg *msgElems;
	int msgElemCount = 0;
	create_protocol_arr(msgElems, msgElemCount, return false;);
	
	return assemble_msg(messagesPtr, 
				 ap_apid,
				 ap_seqnum,
				 MSGTYPE_KEEPALIVE_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

static void APKeepAliveHandler(struct uloop_timeout *t) 
{
	protocol_msg sendMsg;
	init_protocol_msg(sendMsg);
	if(!(APAssembleKeepAliveRequest(&sendMsg))) {
		log_e("Failed to assemble Keep Alive Request");
		uloop_end();
		return;
	}
	log_i("Send Keep Alive Request");
	if(!(send_udp(sendMsg))) {
		log_e("Failed to send Keep Alive Request");
		uloop_end();
		return;
	}

	init_protocol_msg_size(retransmitMsg, sendMsg.offset, log_e("Failed to init Retransmit Message"); uloop_end(); return;);
	store_msg(&retransmitMsg, &sendMsg);
	retransmitMsg.type = MSGTYPE_KEEPALIVE_REQUEST; //easy to match response
	free_protocol_msg(sendMsg);
	
	retransmitInterval = 3;
	retransmitCount = 0;
	uloop_timeout_set(&tRetransmit, retransmitInterval * 1000);
}

bool APParseKeepAliveResponse()
{
	uloop_timeout_cancel(&tKeepAlive);
	uloop_timeout_set(&tKeepAlive, keepalive_interval * 1000);
	log_i("Accept Keep Alive Response");
	return true;
}

bool APAssembleUnregisterResponse(protocol_msg *messagesPtr)
{
	if(messagesPtr == NULL) return false;
	
	protocol_msg *msgElems;
	int msgElemCount = 0;
	create_protocol_arr(msgElems, msgElemCount, return false;);
	
	return assemble_msg(messagesPtr, 
				 ap_apid,
				 controller_seqnum,
				 MSGTYPE_UNREGISTER_RESPONSE,
				 msgElems,
				 msgElemCount
	);
}

bool APAssembleSystemResponse(protocol_msg *messagesPtr)
{
	if(messagesPtr == NULL) return false;
	
	protocol_msg *msgElems;
	int msgElemCount = 0;
	create_protocol_arr(msgElems, msgElemCount, return false;);
	
	return assemble_msg(messagesPtr, 
				 ap_apid,
				 controller_seqnum,
				 MSGTYPE_SYSTEM_RESPONSE,
				 msgElems,
				 msgElemCount
	);
}

bool APParseSystemRequest(protocol_msg *completeMsg, u16 msg_len)
{
	log_i("Parse System Request");

	u8 command;
	u16 elemFlag = 0;

	/* parse message elements */
	while(completeMsg->offset < msg_len) 
	{
		u16 type = 0;
		u16 len = 0;

		parse_msgelem(completeMsg, &type, &len);
		// log_d(3, "Parsing Message Element: %u, len: %u", type, len);
		switch(type) 
		{
			case MSGELEMTYPE_SYSTEM_COMMAND:
				if(elemFlag & 0x01) {
					parse_repeated_msgelem(completeMsg, len);
					log_e("Repeated Message Element");
					break;
				}

				if(!(parse_sys_cmd(completeMsg, len, &command)))
					return false;

				elemFlag |= 0x01;
				break;
			default:
				parse_unrecognized_msgelem(completeMsg, len);
				log_e("Unrecognized Message Element");
				break;
		}
	}
	if(elemFlag != 0x01) { //incomplete message
		log_e("Incomplete Message Element in System Request");
		return false;
	}

	log_i("Accept System Request");

	switch(command)
	{
		case SYSTEM_WLAN_DOWN:
			system("wifi down");
			log_d(3, "WLAN Down");
			break;
		case SYSTEM_WLAN_UP:
			system("wifi up");
			log_d(3, "WLAN Up");
			break;
		case SYSTEM_WLAN_RESTART:
			system("wifi restart");
			log_d(3, "Restart WLAN");
			break;
		case SYSTEM_NETWORK_RESTART:
			system("/etc/init.d/network restart");
			log_d(3, "Restart network");
			break;
	}
	return true;
}

bool APAssembleConfigurationUpdateResponse(protocol_msg *messagesPtr)
{
	if(messagesPtr == NULL) return false;
	
	protocol_msg *msgElems;
	int msgElemCount = 0;
	create_protocol_arr(msgElems, msgElemCount, return false;);
	
	return assemble_msg(messagesPtr, 
				 ap_apid,
				 controller_seqnum,
				 MSGTYPE_CONFIGURATION_UPDATE_RESPONSE,
				 msgElems,
				 msgElemCount
	);
}

bool APParseConfigurationUpdateRequest(protocol_msg *completeMsg, u16 msg_len)
{
	log_i("Parse Configuration Update Request");

	/* parse message elements */
	while(completeMsg->offset < msg_len) 
	{
		u16 type = 0;
		u16 len = 0;

		parse_msgelem(completeMsg, &type, &len);
		// log_d(3, "Parsing Message Element: %u, len: %u", type, len);
		switch(type) 
		{
			case MSGELEMTYPE_SSID:
			{
				char *ssid;
				if(!(parse_ssid(completeMsg, len, &ssid)))
					return false;
				wlconf->set_ssid(wlconf, ssid);
				free_object(ssid);
				break;
			}
			case MSGELEMTYPE_CHANNEL:
			{
				u8 channel;
				if(!(parse_channel(completeMsg, len, &channel)))
					return false;
				wlconf->set_channel(wlconf, channel);
				break;
			}
			case MSGELEMTYPE_HARDWARE_MODE:
			{
				u8 hw_mode;
				if(!(parse_hwmode(completeMsg, len, &hw_mode)))
					return false;
				switch(hw_mode)
				{
					case HWMODE_A:
						wlconf->set_hwmode(wlconf, ONLY_A);
						break;
					case HWMODE_B:
						wlconf->set_hwmode(wlconf, ONLY_B);
						break;
					case HWMODE_G:
						wlconf->set_hwmode(wlconf, ONLY_G);
						break;
					case HWMODE_N:
						wlconf->set_hwmode(wlconf, ONLY_N);
						break;
					default:
						break;
				}
				break;
			}
			case MSGELEMTYPE_SUPPRESS_SSID:
			{
				u8 suppress;
				if(!(parse_hide_ssid(completeMsg, len, &suppress)))
					return false;
				if(suppress == SUPPRESS_SSID_DISABLED) wlconf->set_ssid_hidden(wlconf, false);
				else wlconf->set_ssid_hidden(wlconf, true);
				break;
			}
			case MSGELEMTYPE_SECURITY_OPTION:
			{
				u8 security;
				if(!(parse_sec_opt(completeMsg, len, &security)))
					return false;
				switch(security)
				{
					case SECURITY_OPEN:
						wlconf->set_encryption(wlconf, NO_ENCRYPTION);
						break;
					case SECURITY_WPA_WPA2_MIXED:
						wlconf->set_encryption(wlconf, WPA_WPA2_MIXED);
						break;
					case SECURITY_WPA2:
						wlconf->set_encryption(wlconf, WPA2_PSK);
						break;
					case SECURITY_WPA:
						wlconf->set_encryption(wlconf, WPA_PSK);
						break;
					default:
						break;
				}
				break;
			}
			case MSGELEMTYPE_MACFILTER_MODE:
			{
				u8 mode;
				if(!(parse_macfilter_mode(completeMsg, len, &mode)))
					return false;
				switch(mode)
				{
					case FILTER_NONE:
						wlconf->set_macfilter(wlconf, MAC_FILTER_NONE);
						break;
					case FILTER_ACCEPT_ONLY:
						wlconf->set_macfilter(wlconf, MAC_FILTER_ALLOW);
						break;
					case FILTER_DENY:
						wlconf->set_macfilter(wlconf, MAC_FILTER_DENY);
						break;
					default:
						break;
				}
				break;
			}
			case MSGELEMTYPE_TX_POWER:
			{
				u8 tx_power;
				if(!(parse_tx_power(completeMsg, len, &tx_power)))
					return false;
				wlconf->set_txpower(wlconf, tx_power);
				break;
			}
			case MSGELEMTYPE_WPA_PWD:
			{
				char *pwd;
				if(!(parse_wpa_pwd(completeMsg, len, &pwd)))
					return false;
				wlconf->set_key(wlconf, pwd);
				free_object(pwd);
				break;
			}
			case MSGELEMTYPE_ADD_MACFILTER:
			{
				int i;
				if(len % 6) {
					log_e("Message Element Malformed in MAC Addr");
					break;
				}
				int num = len / 6;
				char **maclist;
				create_array(maclist, len, char*, return false;)
				if(!(parse_mac_list(completeMsg, len, &maclist)))
					return false;
				for(i = 0; i < num; i++) {
					wlconf->add_macfilterlist(wlconf, maclist[i]);
					log_d(5, "Add MAC Filter List: %s", maclist[i]);
					free_object(maclist[i]);
				}
				free_object(maclist);
				break;
			}
			case MSGELEMTYPE_DEL_MACFILTER:
			{
				int i;
				if(len % 6) {
					log_e("Message Element Malformed in MAC Addr");
					break;
				}
				int num = len / 6;
				char **maclist;
				create_array(maclist, len, char*, return false;)
				if(!(parse_mac_list(completeMsg, len, &maclist)))
					return false;
				for(i = 0; i < num; i++) {
					wlconf->del_macfilterlist(wlconf, maclist[i]);
					log_d(5, "Delete MAC Filter List: %s", maclist[i]);
					free_object(maclist[i]);
				}
				free_object(maclist);
				break;
			}
			case MSGELEMTYPE_CLEAR_MACFILTER:
			{
				wlconf->clear_macfilterlist(wlconf);
				log_d(5, "Clear MAC Filter List");
				break;
			}
			case MSGELEMTYPE_RESET_MACFILTER:
			{
				int i;
				if(len % 6) {
					log_e("Message Element Malformed in MAC Addr");
					break;
				}
				int num = len / 6;
				char **maclist;
				create_array(maclist, len, char*, return false;)
				if(!(parse_mac_list(completeMsg, len, &maclist)))
					return false;
				wlconf->clear_macfilterlist(wlconf);
				for(i = 0; i < num; i++) {
					wlconf->add_macfilterlist(wlconf, maclist[i]);
					log_d(5, "Reset & Add MAC Filter List: %s", maclist[i]);
					free_object(maclist[i]);
				}
				free_object(maclist);
				break;
			}
			default:
				parse_unrecognized_msgelem(completeMsg, len);
				log_e("Unrecognized Message Element");
				break;
		}
	}

	log_i("Accept Configuration Update Request");

	wlconf->change_commit(wlconf);
	system("wifi restart");
	log_d(3, "Restart WLAN");
	return true;
}

bool APAssembleConfigurationResponse(protocol_msg *messagesPtr, u8* list, int listSize)
{
	int k = -1;
	int pos = 0;
	u16 desiredType = 0;
	if(messagesPtr == NULL) return false;
	
	protocol_msg *msgElems;
	int msgElemCount = listSize;
	create_protocol_arr(msgElems, msgElemCount, return false;);

	wlconf->update(wlconf);
	while(pos < listSize * 2)
	{
		copy_memory(&desiredType, list + pos, 2);

		switch(desiredType)
		{
			case MSGELEMTYPE_SSID:
				if(!(assemble_ssid(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_CHANNEL:
				if(!(assemble_channel(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_HARDWARE_MODE:
				if(!(assemble_hwmode(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_SUPPRESS_SSID:
				if(!(assemble_hide_ssid(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_SECURITY_OPTION:
				if(!(assemble_sec_opt(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_MACFILTER_MODE:
				if(!(assemble_macfilter_mode(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_MACFILTER_LIST:
				if(!(assemble_macfilter_list(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_TX_POWER:
				if(!(assemble_tx_power(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_WPA_PWD:
				if(!(assemble_wpa_pwd(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			default:
				log_e("Unknown desired configuration type");
				break;
		}

		pos += 2;
	}

	//free_object(list);
	
	return assemble_msg(messagesPtr, 
				 ap_apid,
				 controller_seqnum,
				 MSGTYPE_CONFIGURATION_RESPONSE,
				 msgElems,
				 msgElemCount
	);
}

bool APParseConfigurationRequest(protocol_msg *completeMsg, u16 msg_len, u8 **listPtr, int *listSize)
{
	log_i("Parse Configuration Request");

	u16 elemFlag = 0;

	/* parse message elements */
	while(completeMsg->offset < msg_len) 
	{
		u16 type = 0;
		u16 len = 0;

		parse_msgelem(completeMsg, &type, &len);
		// log_d(3, "Parsing Message Element: %u, len: %u", type, len);
		switch(type) 
		{
			case MSGELEMTYPE_DESIRED_CONF_LIST:
				if(elemFlag & 0x01) {
					parse_repeated_msgelem(completeMsg, len);
					log_e("Repeated Message Element");
					break;
				}
				*listSize = len / 2; //each type takes 2 bytes
				log_d(5, "Parse Desired Conf List Size: %d", *listSize);

				if(!(parse_desired_conf_list(completeMsg, len, listPtr)))
					return false;

				elemFlag |= 0x01;
				break;
			default:
				parse_unrecognized_msgelem(completeMsg, len);
				log_e("Unrecognized Message Element");
				break;
		}
	}
	if(elemFlag != 0x01) { //incomplete message
		log_e("Incomplete Message Element in Configuration Request");
		return false;
	}

	if(*listSize == 0) {
		log_e("There is no requested configuration");
	}
	log_i("Accept Configuration Request");

	return true;
}

bool APReceiveMessageInRunState() 
{
	char buf[BUFFER_SIZE];
	struct sockaddr_in addr;
	int readBytes;
	u32 recvAddr;

	/* receive the datagram */
	if(!(recv_udp(buf,
					 BUFFER_SIZE - 1,
					 &addr,
					 &readBytes))) {
		log_e("Receive Message in run state failed");
		return false;
	}

	recvAddr = ntohl(addr.sin_addr.s_addr);
	/* verify the source of the message */
	if(recvAddr != controller_ip) {
		log_e("Message from the illegal source address");
		return false;
	}

	log_d(3, "Receive Message in run state");

	header_val controlVal;
	protocol_msg completeMsg;
	completeMsg.msg = buf;
	completeMsg.offset = 0;

	if(!(parse_header(&completeMsg, &controlVal))) {
		log_e("Failed to parse header");
		return false;
	}

	/* not as expected */
	if(controlVal.version != CURRENT_VERSION || controlVal.type != TYPE_CONTROL) {
		log_e("ACAMP version or type is not Expected");
		return false;
	}
	if(controlVal.apid != ap_apid) {
		log_e("The apid in message is different from the one in message header");
		return false;
	}

	int is_req = controlVal.msg_type % 2; //odd type indicates the request message

	if(is_req) {
		if(controlVal.seq_num == controller_seqnum - 1) {
			log_d(3, "Receive a message that has been responsed");
			if(cacheMsg.type == 0 || controlVal.msg_type + 1 != cacheMsg.type) {
				log_e("The received message does not match the cache message");
				return false;
			}

			log_d(3, "Send Cache Message");
			if(!(send_udp(cacheMsg))) {
				log_e("Failed to send Cache Message");
				return false;
			}

			uloop_timeout_cancel(&tKeepAlive);
			uloop_timeout_set(&tKeepAlive, keepalive_interval * 1000);
			return false;
		}
		if(controlVal.seq_num != controller_seqnum) {
			if(controlVal.seq_num < controller_seqnum)
				log_e("The serial number of the message is expired");
			else
				log_e("The serial number of the message is invalid");
			return false;
		}

		switch(controlVal.msg_type)
		{
			case MSGTYPE_CONFIGURATION_REQUEST:
			{
				u8* list = NULL;
				int listSize; 
				if(!(APParseConfigurationRequest(&completeMsg, controlVal.msg_len, &list, &listSize))) {
					return false;
				}

				protocol_msg responseMsg;
				init_protocol_msg(responseMsg);
				if(!(APAssembleConfigurationResponse(&responseMsg, list, listSize))) {
					log_e("Failed to assemble Configuration Response");
					return false;
				}
				log_i("Send Configuration Response");
				if(!(send_udp(responseMsg))) {
					log_e("Failed to send Configuration Response");
					return false;
				}
				free_object(list);

				free_protocol_msg(cacheMsg);
				init_protocol_msg_size(cacheMsg, responseMsg.offset, return false;);
				store_msg(&cacheMsg, &responseMsg);
				cacheMsg.type = MSGTYPE_CONFIGURATION_RESPONSE; //easy to match request
				free_protocol_msg(responseMsg);

				controller_seqnum_inc();
				uloop_timeout_cancel(&tKeepAlive);
				uloop_timeout_set(&tKeepAlive, keepalive_interval * 1000);
				break;
			}
			case MSGTYPE_CONFIGURATION_UPDATE_REQUEST:
			{
				if(!(APParseConfigurationUpdateRequest(&completeMsg, controlVal.msg_len))) {
					return false;
				}

				protocol_msg responseMsg;
				init_protocol_msg(responseMsg);
				if(!(APAssembleConfigurationUpdateResponse(&responseMsg))) {
					log_e("Failed to assemble Configuration Update Response");
					return false;
				}
				log_i("Send Configuration Update Response");
				if(!(send_udp(responseMsg))) {
					log_e("Failed to send Configuration Update Response");
					return false;
				}

				free_protocol_msg(cacheMsg);
				init_protocol_msg_size(cacheMsg, responseMsg.offset, return false;);
				store_msg(&cacheMsg, &responseMsg);
				cacheMsg.type = MSGTYPE_CONFIGURATION_UPDATE_RESPONSE; //easy to match request
				free_protocol_msg(responseMsg);

				controller_seqnum_inc();
				uloop_timeout_cancel(&tKeepAlive);
				uloop_timeout_set(&tKeepAlive, keepalive_interval * 1000);
				break;
			}
			case MSGTYPE_SYSTEM_REQUEST:
			{
				if(!(APParseSystemRequest(&completeMsg, controlVal.msg_len))) {
					return false;
				}

				protocol_msg responseMsg;
				init_protocol_msg(responseMsg);
				if(!(APAssembleSystemResponse(&responseMsg))) {
					log_e("Failed to assemble System Response");
					return false;
				}
				log_i("Send System Response");
				if(!(send_udp(responseMsg))) {
					log_e("Failed to send System Response");
					return false;
				}

				free_protocol_msg(cacheMsg);
				init_protocol_msg_size(cacheMsg, responseMsg.offset, return false;);
				store_msg(&cacheMsg, &responseMsg);
				cacheMsg.type = MSGTYPE_SYSTEM_RESPONSE; //easy to match request
				free_protocol_msg(responseMsg);

				controller_seqnum_inc();
				uloop_timeout_cancel(&tKeepAlive);
				uloop_timeout_set(&tKeepAlive, keepalive_interval * 1000);
				break;
			}
			case MSGTYPE_UNREGISTER_REQUEST:
			{
				log_i("Accept Unregister Request");
				uloop_end(); //goto AP_DOWN

				protocol_msg responseMsg;
				init_protocol_msg(responseMsg);
				if(!(APAssembleUnregisterResponse(&responseMsg))) {
					log_e("Failed to assemble Unregister Response");
					return false;
				}
				log_i("Send Unregister Response");
				if(!(send_udp(responseMsg))) {
					log_e("Failed to send Unregister Response");
					return false;
				}
				free_protocol_msg(cacheMsg);
				uloop_timeout_cancel(&tKeepAlive);
				break;
			}
			default:
				return false;
		}


	} else {
		if(controlVal.seq_num != ap_seqnum) {
			if(controlVal.seq_num < ap_seqnum)
				log_e("The serial number of the message is expired");
			else
				log_e("The serial number of the message is invalid");
			return false;
		}
		if(retransmitMsg.type == 0 || controlVal.msg_type != retransmitMsg.type + 1) {
			log_e("The received message does not match the send message");
			return false;
		}
		switch(controlVal.msg_type)
		{
			case MSGTYPE_KEEPALIVE_RESPONSE:
				if(!(APParseKeepAliveResponse())) {
					return false;
				}
				uloop_timeout_cancel(&tRetransmit);
				free_protocol_msg(retransmitMsg);
				ap_seqnum_inc();
				break;
			default:
				return false;
		}
	}

	return true;
}

static void APEvents(struct uloop_fd *event_fd, unsigned int events)
{
	int sockfd = event_fd->fd;
	if(sockfd != ap_socket) {
		log_e("Received an event from unknown source");
	}
	(APReceiveMessageInRunState());
}

state APEnterRun() 
{
	log_i("");	
	log_i("######### Run State #########");

	free_protocol_msg(cacheMsg);
	free_protocol_msg(retransmitMsg);


	uloop_init();

	tRetransmit.cb = APRretransmitHandler;
	tKeepAlive.cb = APKeepAliveHandler;
	uloop_timeout_set(&tKeepAlive, 100); //send a keep alive req soon

	fdSocket.fd = ap_socket;
	fdSocket.cb = APEvents;
	uloop_fd_add(&fdSocket, ULOOP_READ);

	uloop_run();

	uloop_done();

	return ENTER_DOWN;
}