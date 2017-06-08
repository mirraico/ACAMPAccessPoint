#include "common.h"
#include "error.h"
#include "log.h"
#include "ap.h"
#include "network.h"

#define MAX_EVENT_NUMBER 1024

struct uloop_fd fdSocket;
struct uloop_timeout tKeepAlive;
struct uloop_timeout tRetransmit;
APProtocolMessage retransmitMsg;
int retransmitInterval;
int retransmitCount;
APProtocolMessage cacheMsg;

static void APRretransmitHandler(struct uloop_timeout *t)
{
	retransmitCount++;
	if(retransmitCount >= 5) {
		log("There is no valid Response for %d times", retransmitCount);
		log("The connection to controller has been interrupted");
		uloop_end();
		return;
	}

	log("There is no valid Response, times = %d, retransmit request", retransmitCount);
	retransmitInterval *= 2;
	if(retransmitInterval > keepalive_interval / 2) {
		retransmitInterval = keepalive_interval / 2;
	}
	uloop_timeout_set(&tRetransmit, retransmitInterval * 1000);
	log_d(5, "Adjust the retransmit interval to %d sec and retransmit", retransmitInterval);
	if(!(APNetworkSend(retransmitMsg))) {
		log_e("Failed to retransmit Request");
		uloop_end();
		return;
	}
}

bool APAssembleKeepAliveRequest(APProtocolMessage *messagesPtr)
{
	int k = -1;
	if(messagesPtr == NULL) return false;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return false;);
	
	return APAssembleControlMessage(messagesPtr, 
				 ap_apid,
				 ap_seqnum,
				 MSGTYPE_KEEPALIVE_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

static void APKeepAliveHandler(struct uloop_timeout *t) 
{
	APProtocolMessage sendMsg;
	AP_INIT_PROTOCOL(sendMsg);
	if(!(APAssembleKeepAliveRequest(&sendMsg))) {
		log_e("Failed to assemble Keep Alive Request");
		uloop_end();
		return;
	}
	log("Send Keep Alive Request");
	if(!(APNetworkSend(sendMsg))) {
		log_e("Failed to send Keep Alive Request");
		uloop_end();
		return;
	}

	AP_INIT_PROTOCOL_MESSAGE(retransmitMsg, sendMsg.offset, log_e("Failed to init Retransmit Message"); uloop_end(); return;);
	APProtocolStoreMessage(&retransmitMsg, &sendMsg);
	retransmitMsg.type = MSGTYPE_KEEPALIVE_REQUEST; //easy to match response
	AP_FREE_PROTOCOL_MESSAGE(sendMsg);
	
	retransmitInterval = 3;
	retransmitCount = 0;
	uloop_timeout_set(&tRetransmit, retransmitInterval * 1000);
}

bool APParseKeepAliveResponse()
{
	uloop_timeout_cancel(&tKeepAlive);
	uloop_timeout_set(&tKeepAlive, keepalive_interval * 1000);
	log("Accept Keep Alive Response");
	return true;
}

bool APAssembleUnregisterResponse(APProtocolMessage *messagesPtr)
{
	if(messagesPtr == NULL) return false;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return false;);
	
	return APAssembleControlMessage(messagesPtr, 
				 ap_apid,
				 controller_seqnum,
				 MSGTYPE_UNREGISTER_RESPONSE,
				 msgElems,
				 msgElemCount
	);
}

bool APAssembleSystemResponse(APProtocolMessage *messagesPtr)
{
	if(messagesPtr == NULL) return false;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return false;);
	
	return APAssembleControlMessage(messagesPtr, 
				 ap_apid,
				 controller_seqnum,
				 MSGTYPE_SYSTEM_RESPONSE,
				 msgElems,
				 msgElemCount
	);
}

bool APParseSystemRequest(APProtocolMessage *completeMsg, u16 msgLen)
{
	log("Parse System Request");

	u8 command;
	u16 elemFlag = 0;

	/* parse message elements */
	while(completeMsg->offset < msgLen) 
	{
		u16 type = 0;
		u16 len = 0;

		APParseFormatMsgElem(completeMsg, &type, &len);
		// log_d(3, "Parsing Message Element: %u, len: %u", type, len);
		switch(type) 
		{
			case MSGELEMTYPE_SYSTEM_COMMAND:
				if(elemFlag & 0x01) {
					APParseRepeatedMsgElem(completeMsg, len);
					log_e("Repeated Message Element");
					break;
				}

				if(!(APParseSystemCommand(completeMsg, len, &command)))
					return false;

				elemFlag |= 0x01;
				break;
			default:
				APParseUnrecognizedMsgElem(completeMsg, len);
				log_e("Unrecognized Message Element");
				break;
		}
	}
	if(elemFlag != 0x01) { //incomplete message
		log_e("Incomplete Message Element in System Request");
		return false;
	}

	log("Accept System Request");

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

bool APAssembleConfigurationUpdateResponse(APProtocolMessage *messagesPtr)
{
	if(messagesPtr == NULL) return false;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return false;);
	
	return APAssembleControlMessage(messagesPtr, 
				 ap_apid,
				 controller_seqnum,
				 MSGTYPE_CONFIGURATION_UPDATE_RESPONSE,
				 msgElems,
				 msgElemCount
	);
}

bool APParseConfigurationUpdateRequest(APProtocolMessage *completeMsg, u16 msgLen)
{
	log("Parse Configuration Update Request");

	/* parse message elements */
	while(completeMsg->offset < msgLen) 
	{
		u16 type = 0;
		u16 len = 0;

		APParseFormatMsgElem(completeMsg, &type, &len);
		// log_d(3, "Parsing Message Element: %u, len: %u", type, len);
		switch(type) 
		{
			case MSGELEMTYPE_SSID:
			{
				char *ssid;
				if(!(APParseSSID(completeMsg, len, &ssid)))
					return false;
				wlconf->set_ssid(wlconf, ssid);
				free_object(ssid);
				break;
			}
			case MSGELEMTYPE_CHANNEL:
			{
				u8 channel;
				if(!(APParseChannel(completeMsg, len, &channel)))
					return false;
				wlconf->set_channel(wlconf, channel);
				break;
			}
			case MSGELEMTYPE_HARDWARE_MODE:
			{
				u8 hw_mode;
				if(!(APParseHardwareMode(completeMsg, len, &hw_mode)))
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
				if(!(APParseSuppressSSID(completeMsg, len, &suppress)))
					return false;
				if(suppress == SUPPRESS_SSID_DISABLED) wlconf->set_ssid_hidden(wlconf, false);
				else wlconf->set_ssid_hidden(wlconf, true);
				break;
			}
			case MSGELEMTYPE_SECURITY_OPTION:
			{
				u8 security;
				if(!(APParseSecurityOption(completeMsg, len, &security)))
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
				if(!(APParseMACFilterMode(completeMsg, len, &mode)))
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
				if(!(APParseTxPower(completeMsg, len, &tx_power)))
					return false;
				wlconf->set_txpower(wlconf, tx_power);
				break;
			}
			case MSGELEMTYPE_WPA_PWD:
			{
				char *pwd;
				if(!(APParseWPAPassword(completeMsg, len, &pwd)))
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
				if(!(APParseMACList(completeMsg, len, &maclist)))
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
				if(!(APParseMACList(completeMsg, len, &maclist)))
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
				if(!(APParseMACList(completeMsg, len, &maclist)))
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
				APParseUnrecognizedMsgElem(completeMsg, len);
				log_e("Unrecognized Message Element");
				break;
		}
	}

	log("Accept Configuration Update Request");

	wlconf->change_commit(wlconf);
	system("wifi restart");
	log_d(3, "Restart WLAN");
	return true;
}

bool APAssembleConfigurationResponse(APProtocolMessage *messagesPtr, u8* list, int listSize)
{
	int k = -1;
	int pos = 0;
	u16 desiredType = 0;
	if(messagesPtr == NULL) return false;
	
	APProtocolMessage *msgElems;
	int msgElemCount = listSize;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return false;);

	wlconf->update(wlconf);
	while(pos < listSize * 2)
	{
		copy_memory(&desiredType, list + pos, 2);

		switch(desiredType)
		{
			case MSGELEMTYPE_SSID:
				if(!(APAssembleSSID(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_CHANNEL:
				if(!(APAssembleChannel(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_HARDWARE_MODE:
				if(!(APAssembleHardwareMode(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_SUPPRESS_SSID:
				if(!(APAssembleSuppressSSID(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_SECURITY_OPTION:
				if(!(APAssembleSecurityOption(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_MACFILTER_MODE:
				if(!(APAssembleMACFilterMode(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_MACFILTER_LIST:
				if(!(APAssembleMACFilterList(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_TX_POWER:
				if(!(APAssembleTxPower(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
					free_object(msgElems);
					return false;
				}
				break;
			case MSGELEMTYPE_WPA_PWD:
				if(!(APAssembleWPAPassword(&(msgElems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
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
	
	return APAssembleControlMessage(messagesPtr, 
				 ap_apid,
				 controller_seqnum,
				 MSGTYPE_CONFIGURATION_RESPONSE,
				 msgElems,
				 msgElemCount
	);
}

bool APParseConfigurationRequest(APProtocolMessage *completeMsg, u16 msgLen, u8 **listPtr, int *listSize)
{
	log("Parse Configuration Request");

	u16 elemFlag = 0;

	/* parse message elements */
	while(completeMsg->offset < msgLen) 
	{
		u16 type = 0;
		u16 len = 0;

		APParseFormatMsgElem(completeMsg, &type, &len);
		// log_d(3, "Parsing Message Element: %u, len: %u", type, len);
		switch(type) 
		{
			case MSGELEMTYPE_DESIRED_CONF_LIST:
				if(elemFlag & 0x01) {
					APParseRepeatedMsgElem(completeMsg, len);
					log_e("Repeated Message Element");
					break;
				}
				*listSize = len / 2; //each type takes 2 bytes
				log_d(5, "Parse Desired Conf List Size: %d", *listSize);

				if(!(APParseDesiredConfList(completeMsg, len, listPtr)))
					return false;

				elemFlag |= 0x01;
				break;
			default:
				APParseUnrecognizedMsgElem(completeMsg, len);
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
	log("Accept Configuration Request");

	return true;
}

bool APReceiveMessageInRunState() 
{
	char buf[BUFFER_SIZE];
	APNetworkAddress addr;
	int readBytes;
	u32 recvAddr;

	/* receive the datagram */
	if(!(APNetworkReceive(buf,
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

	APHeaderVal controlVal;
	APProtocolMessage completeMsg;
	completeMsg.msg = buf;
	completeMsg.offset = 0;

	if(!(APParseControlHeader(&completeMsg, &controlVal))) {
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

	int is_req = controlVal.msgType % 2; //odd type indicates the request message

	if(is_req) {
		if(controlVal.seqNum == controller_seqnum - 1) {
			log_d(3, "Receive a message that has been responsed");
			if(cacheMsg.type == 0 || controlVal.msgType + 1 != cacheMsg.type) {
				log_e("The received message does not match the cache message");
				return false;
			}

			log_d(3, "Send Cache Message");
			if(!(APNetworkSend(cacheMsg))) {
				log_e("Failed to send Cache Message");
				return false;
			}

			uloop_timeout_cancel(&tKeepAlive);
			uloop_timeout_set(&tKeepAlive, keepalive_interval * 1000);
			return false;
		}
		if(controlVal.seqNum != controller_seqnum) {
			if(controlVal.seqNum < controller_seqnum)
				log_e("The serial number of the message is expired");
			else
				log_e("The serial number of the message is invalid");
			return false;
		}

		switch(controlVal.msgType)
		{
			case MSGTYPE_CONFIGURATION_REQUEST:
			{
				u8* list = NULL;
				int listSize; 
				if(!(APParseConfigurationRequest(&completeMsg, controlVal.msgLen, &list, &listSize))) {
					return false;
				}

				APProtocolMessage responseMsg;
				AP_INIT_PROTOCOL(responseMsg);
				if(!(APAssembleConfigurationResponse(&responseMsg, list, listSize))) {
					log_e("Failed to assemble Configuration Response");
					return false;
				}
				log("Send Configuration Response");
				if(!(APNetworkSend(responseMsg))) {
					log_e("Failed to send Configuration Response");
					return false;
				}
				free_object(list);

				AP_FREE_PROTOCOL_MESSAGE(cacheMsg);
				AP_INIT_PROTOCOL_MESSAGE(cacheMsg, responseMsg.offset, return false;);
				APProtocolStoreMessage(&cacheMsg, &responseMsg);
				cacheMsg.type = MSGTYPE_CONFIGURATION_RESPONSE; //easy to match request
				AP_FREE_PROTOCOL_MESSAGE(responseMsg);

				controller_seqnum_inc();
				uloop_timeout_cancel(&tKeepAlive);
				uloop_timeout_set(&tKeepAlive, keepalive_interval * 1000);
				break;
			}
			case MSGTYPE_CONFIGURATION_UPDATE_REQUEST:
			{
				if(!(APParseConfigurationUpdateRequest(&completeMsg, controlVal.msgLen))) {
					return false;
				}

				APProtocolMessage responseMsg;
				AP_INIT_PROTOCOL(responseMsg);
				if(!(APAssembleConfigurationUpdateResponse(&responseMsg))) {
					log_e("Failed to assemble Configuration Update Response");
					return false;
				}
				log("Send Configuration Update Response");
				if(!(APNetworkSend(responseMsg))) {
					log_e("Failed to send Configuration Update Response");
					return false;
				}

				AP_FREE_PROTOCOL_MESSAGE(cacheMsg);
				AP_INIT_PROTOCOL_MESSAGE(cacheMsg, responseMsg.offset, return false;);
				APProtocolStoreMessage(&cacheMsg, &responseMsg);
				cacheMsg.type = MSGTYPE_CONFIGURATION_UPDATE_RESPONSE; //easy to match request
				AP_FREE_PROTOCOL_MESSAGE(responseMsg);

				controller_seqnum_inc();
				uloop_timeout_cancel(&tKeepAlive);
				uloop_timeout_set(&tKeepAlive, keepalive_interval * 1000);
				break;
			}
			case MSGTYPE_SYSTEM_REQUEST:
			{
				if(!(APParseSystemRequest(&completeMsg, controlVal.msgLen))) {
					return false;
				}

				APProtocolMessage responseMsg;
				AP_INIT_PROTOCOL(responseMsg);
				if(!(APAssembleSystemResponse(&responseMsg))) {
					log_e("Failed to assemble System Response");
					return false;
				}
				log("Send System Response");
				if(!(APNetworkSend(responseMsg))) {
					log_e("Failed to send System Response");
					return false;
				}

				AP_FREE_PROTOCOL_MESSAGE(cacheMsg);
				AP_INIT_PROTOCOL_MESSAGE(cacheMsg, responseMsg.offset, return false;);
				APProtocolStoreMessage(&cacheMsg, &responseMsg);
				cacheMsg.type = MSGTYPE_SYSTEM_RESPONSE; //easy to match request
				AP_FREE_PROTOCOL_MESSAGE(responseMsg);

				controller_seqnum_inc();
				uloop_timeout_cancel(&tKeepAlive);
				uloop_timeout_set(&tKeepAlive, keepalive_interval * 1000);
				break;
			}
			case MSGTYPE_UNREGISTER_REQUEST:
			{
				log("Accept Unregister Request");
				uloop_end(); //goto AP_DOWN

				APProtocolMessage responseMsg;
				AP_INIT_PROTOCOL(responseMsg);
				if(!(APAssembleUnregisterResponse(&responseMsg))) {
					log_e("Failed to assemble Unregister Response");
					return false;
				}
				log("Send Unregister Response");
				if(!(APNetworkSend(responseMsg))) {
					log_e("Failed to send Unregister Response");
					return false;
				}
				AP_FREE_PROTOCOL_MESSAGE(cacheMsg);
				uloop_timeout_cancel(&tKeepAlive);
				break;
			}
			default:
				return false;
		}


	} else {
		if(controlVal.seqNum != ap_seqnum) {
			if(controlVal.seqNum < ap_seqnum)
				log_e("The serial number of the message is expired");
			else
				log_e("The serial number of the message is invalid");
			return false;
		}
		if(retransmitMsg.type == 0 || controlVal.msgType != retransmitMsg.type + 1) {
			log_e("The received message does not match the send message");
			return false;
		}
		switch(controlVal.msgType)
		{
			case MSGTYPE_KEEPALIVE_RESPONSE:
				if(!(APParseKeepAliveResponse())) {
					return false;
				}
				uloop_timeout_cancel(&tRetransmit);
				AP_FREE_PROTOCOL_MESSAGE(retransmitMsg);
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
	if(sockfd != gSocket) {
		log_e("Received an event from unknown source");
	}
	(APReceiveMessageInRunState());
}

APStateTransition APEnterRun() 
{
	log("");	
	log("######### Run State #########");

	AP_FREE_PROTOCOL_MESSAGE(cacheMsg);
	AP_FREE_PROTOCOL_MESSAGE(retransmitMsg);


	uloop_init();

	tRetransmit.cb = APRretransmitHandler;
	tKeepAlive.cb = APKeepAliveHandler;
	uloop_timeout_set(&tKeepAlive, 100); //send a keep alive req soon

	fdSocket.fd = gSocket;
	fdSocket.cb = APEvents;
	uloop_fd_add(&fdSocket, ULOOP_READ);

	uloop_run();

	uloop_done();

	return AP_ENTER_DOWN;
}