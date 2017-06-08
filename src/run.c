#include "common.h"
#include "error.h"
#include "log.h"
#include "ap.h"
#include "network.h"

#define MAX_EVENT_NUMBER 1024

struct uloop_fd fd_socket;
struct uloop_timeout timer_keepalive;
struct uloop_timeout timer_retransmit;
protocol_msg retransmit_msg;
int retransmit_interval;
int retransmit_cnt;
protocol_msg cache_msg;

static void retransmit_handler(struct uloop_timeout *t)
{
	retransmit_cnt++;
	if(retransmit_cnt >= 5) {
		log_i("There is no valid Response for %d times", retransmit_cnt);
		log_i("The connection to controller has been interrupted");
		uloop_end();
		return;
	}

	log_i("There is no valid Response, times = %d, retransmit request", retransmit_cnt);
	retransmit_interval *= 2;
	if(retransmit_interval > keepalive_interval / 2) {
		retransmit_interval = keepalive_interval / 2;
	}
	uloop_timeout_set(&timer_retransmit, retransmit_interval * 1000);
	log_d(5, "Adjust the retransmit interval to %d sec and retransmit", retransmit_interval);
	if(!(send_udp(retransmit_msg))) {
		log_e("Failed to retransmit Request");
		uloop_end();
		return;
	}
}

bool assemble_keepalive_req(protocol_msg *msg_p)
{
	int k = -1;
	if(msg_p == NULL) return false;
	
	protocol_msg *msgelems;
	int msgelem_cnt = 0;
	create_protocol_arr(msgelems, msgelem_cnt, return false;);
	
	return assemble_msg(msg_p, 
				 ap_apid,
				 ap_seqnum,
				 MSGTYPE_KEEPALIVE_REQUEST,
				 msgelems,
				 msgelem_cnt
	);
}

static void keepalive_handler(struct uloop_timeout *t) 
{
	protocol_msg send_msg;
	init_protocol_msg(send_msg);
	if(!(assemble_keepalive_req(&send_msg))) {
		log_e("Failed to assemble Keep Alive Request");
		uloop_end();
		return;
	}
	log_i("Send Keep Alive Request");
	if(!(send_udp(send_msg))) {
		log_e("Failed to send Keep Alive Request");
		uloop_end();
		return;
	}

	init_protocol_msg_size(retransmit_msg, send_msg.offset, log_e("Failed to init Retransmit Message"); uloop_end(); return;);
	store_msg(&retransmit_msg, &send_msg);
	retransmit_msg.type = MSGTYPE_KEEPALIVE_REQUEST; //easy to match response
	free_protocol_msg(send_msg);
	
	retransmit_interval = 3;
	retransmit_cnt = 0;
	uloop_timeout_set(&timer_retransmit, retransmit_interval * 1000);
}

bool parse_keepalive_resp()
{
	uloop_timeout_cancel(&timer_keepalive);
	uloop_timeout_set(&timer_keepalive, keepalive_interval * 1000);
	log_i("Accept Keep Alive Response");
	return true;
}

bool assemble_unregister_resp(protocol_msg *msg_p)
{
	if(msg_p == NULL) return false;
	
	protocol_msg *msgelems;
	int msgelem_cnt = 0;
	create_protocol_arr(msgelems, msgelem_cnt, return false;);
	
	return assemble_msg(msg_p, 
				 ap_apid,
				 controller_seqnum,
				 MSGTYPE_UNREGISTER_RESPONSE,
				 msgelems,
				 msgelem_cnt
	);
}

bool assemble_system_resp(protocol_msg *msg_p)
{
	if(msg_p == NULL) return false;
	
	protocol_msg *msgelems;
	int msgelem_cnt = 0;
	create_protocol_arr(msgelems, msgelem_cnt, return false;);
	
	return assemble_msg(msg_p, 
				 ap_apid,
				 controller_seqnum,
				 MSGTYPE_SYSTEM_RESPONSE,
				 msgelems,
				 msgelem_cnt
	);
}

bool parse_system_req(protocol_msg *complete_msg, u16 msg_len)
{
	log_i("Parse System Request");

	u8 command;
	u16 elem_flag = 0;

	/* parse message elements */
	while(complete_msg->offset < msg_len) 
	{
		u16 type = 0;
		u16 len = 0;

		parse_msgelem(complete_msg, &type, &len);
		// log_d(3, "Parsing Message Element: %u, len: %u", type, len);
		switch(type) 
		{
			case MSGELEMTYPE_SYSTEM_COMMAND:
				if(elem_flag & 0x01) {
					parse_repeated_msgelem(complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}

				if(!(parse_sys_cmd(complete_msg, len, &command)))
					return false;

				elem_flag |= 0x01;
				break;
			default:
				parse_unrecognized_msgelem(complete_msg, len);
				log_e("Unrecognized Message Element");
				break;
		}
	}
	if(elem_flag != 0x01) { //incomplete message
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

bool assemble_conf_update_resp(protocol_msg *msg_p)
{
	if(msg_p == NULL) return false;
	
	protocol_msg *msgelems;
	int msgelem_cnt = 0;
	create_protocol_arr(msgelems, msgelem_cnt, return false;);
	
	return assemble_msg(msg_p, 
				 ap_apid,
				 controller_seqnum,
				 MSGTYPE_CONFIGURATION_UPDATE_RESPONSE,
				 msgelems,
				 msgelem_cnt
	);
}

bool parse_conf_update_req(protocol_msg *complete_msg, u16 msg_len)
{
	log_i("Parse Configuration Update Request");

	/* parse message elements */
	while(complete_msg->offset < msg_len) 
	{
		u16 type = 0;
		u16 len = 0;

		parse_msgelem(complete_msg, &type, &len);
		// log_d(3, "Parsing Message Element: %u, len: %u", type, len);
		switch(type) 
		{
			case MSGELEMTYPE_SSID:
			{
				char *ssid;
				if(!(parse_ssid(complete_msg, len, &ssid)))
					return false;
				wlconf->set_ssid(wlconf, ssid);
				free_object(ssid);
				break;
			}
			case MSGELEMTYPE_CHANNEL:
			{
				u8 channel;
				if(!(parse_channel(complete_msg, len, &channel)))
					return false;
				wlconf->set_channel(wlconf, channel);
				break;
			}
			case MSGELEMTYPE_HARDWARE_MODE:
			{
				u8 hw_mode;
				if(!(parse_hwmode(complete_msg, len, &hw_mode)))
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
				if(!(parse_hide_ssid(complete_msg, len, &suppress)))
					return false;
				if(suppress == SUPPRESS_SSID_DISABLED) wlconf->set_ssid_hidden(wlconf, false);
				else wlconf->set_ssid_hidden(wlconf, true);
				break;
			}
			case MSGELEMTYPE_SECURITY_OPTION:
			{
				u8 security;
				if(!(parse_sec_opt(complete_msg, len, &security)))
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
				if(!(parse_macfilter_mode(complete_msg, len, &mode)))
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
				if(!(parse_tx_power(complete_msg, len, &tx_power)))
					return false;
				wlconf->set_txpower(wlconf, tx_power);
				break;
			}
			case MSGELEMTYPE_WPA_PWD:
			{
				char *pwd;
				if(!(parse_wpa_pwd(complete_msg, len, &pwd)))
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
				if(!(parse_mac_list(complete_msg, len, &maclist)))
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
				if(!(parse_mac_list(complete_msg, len, &maclist)))
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
				if(!(parse_mac_list(complete_msg, len, &maclist)))
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
				parse_unrecognized_msgelem(complete_msg, len);
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

bool assemble_conf_resp(protocol_msg *msg_p, u8* list, int listSize)
{
	int k = -1;
	int pos = 0;
	u16 desiredType = 0;
	if(msg_p == NULL) return false;
	
	protocol_msg *msgelems;
	int msgelem_cnt = listSize;
	create_protocol_arr(msgelems, msgelem_cnt, return false;);

	wlconf->update(wlconf);
	while(pos < listSize * 2)
	{
		copy_memory(&desiredType, list + pos, 2);

		switch(desiredType)
		{
			case MSGELEMTYPE_SSID:
				if(!(assemble_ssid(&(msgelems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgelems[i]);}
					free_object(msgelems);
					return false;
				}
				break;
			case MSGELEMTYPE_CHANNEL:
				if(!(assemble_channel(&(msgelems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgelems[i]);}
					free_object(msgelems);
					return false;
				}
				break;
			case MSGELEMTYPE_HARDWARE_MODE:
				if(!(assemble_hwmode(&(msgelems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgelems[i]);}
					free_object(msgelems);
					return false;
				}
				break;
			case MSGELEMTYPE_SUPPRESS_SSID:
				if(!(assemble_hide_ssid(&(msgelems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgelems[i]);}
					free_object(msgelems);
					return false;
				}
				break;
			case MSGELEMTYPE_SECURITY_OPTION:
				if(!(assemble_sec_opt(&(msgelems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgelems[i]);}
					free_object(msgelems);
					return false;
				}
				break;
			case MSGELEMTYPE_MACFILTER_MODE:
				if(!(assemble_macfilter_mode(&(msgelems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgelems[i]);}
					free_object(msgelems);
					return false;
				}
				break;
			case MSGELEMTYPE_MACFILTER_LIST:
				if(!(assemble_macfilter_list(&(msgelems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgelems[i]);}
					free_object(msgelems);
					return false;
				}
				break;
			case MSGELEMTYPE_TX_POWER:
				if(!(assemble_tx_power(&(msgelems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgelems[i]);}
					free_object(msgelems);
					return false;
				}
				break;
			case MSGELEMTYPE_WPA_PWD:
				if(!(assemble_wpa_pwd(&(msgelems[++k])))) {
					int i;
					for(i = 0; i <= k; i++) { free_protocol_msg(msgelems[i]);}
					free_object(msgelems);
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
	
	return assemble_msg(msg_p, 
				 ap_apid,
				 controller_seqnum,
				 MSGTYPE_CONFIGURATION_RESPONSE,
				 msgelems,
				 msgelem_cnt
	);
}

bool parse_conf_req(protocol_msg *complete_msg, u16 msg_len, u8 **listPtr, int *listSize)
{
	log_i("Parse Configuration Request");

	u16 elem_flag = 0;

	/* parse message elements */
	while(complete_msg->offset < msg_len) 
	{
		u16 type = 0;
		u16 len = 0;

		parse_msgelem(complete_msg, &type, &len);
		// log_d(3, "Parsing Message Element: %u, len: %u", type, len);
		switch(type) 
		{
			case MSGELEMTYPE_DESIRED_CONF_LIST:
				if(elem_flag & 0x01) {
					parse_repeated_msgelem(complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}
				*listSize = len / 2; //each type takes 2 bytes
				log_d(5, "Parse Desired Conf List Size: %d", *listSize);

				if(!(parse_desired_conf_list(complete_msg, len, listPtr)))
					return false;

				elem_flag |= 0x01;
				break;
			default:
				parse_unrecognized_msgelem(complete_msg, len);
				log_e("Unrecognized Message Element");
				break;
		}
	}
	if(elem_flag != 0x01) { //incomplete message
		log_e("Incomplete Message Element in Configuration Request");
		return false;
	}

	if(*listSize == 0) {
		log_e("There is no requested configuration");
	}
	log_i("Accept Configuration Request");

	return true;
}

bool msg_handle_in_run() 
{
	char buf[BUFFER_SIZE];
	struct sockaddr_in addr;
	int read_bytes;
	u32 recv_addr;

	/* receive the datagram */
	if(!(recv_udp(buf,
					 BUFFER_SIZE - 1,
					 &addr,
					 &read_bytes))) {
		log_e("Receive Message in run state failed");
		return false;
	}

	recv_addr = ntohl(addr.sin_addr.s_addr);
	/* verify the source of the message */
	if(recv_addr != controller_ip) {
		log_e("Message from the illegal source address");
		return false;
	}

	log_d(3, "Receive Message in run state");

	header_val control_val;
	protocol_msg complete_msg;
	complete_msg.msg = buf;
	complete_msg.offset = 0;

	if(!(parse_header(&complete_msg, &control_val))) {
		log_e("Failed to parse header");
		return false;
	}

	/* not as expected */
	if(control_val.version != CURRENT_VERSION || control_val.type != TYPE_CONTROL) {
		log_e("ACAMP version or type is not Expected");
		return false;
	}
	if(control_val.apid != ap_apid) {
		log_e("The apid in message is different from the one in message header");
		return false;
	}

	int is_req = control_val.msg_type % 2; //odd type indicates the request message

	if(is_req) {
		if(control_val.seq_num == controller_seqnum - 1) {
			log_d(3, "Receive a message that has been responsed");
			if(cache_msg.type == 0 || control_val.msg_type + 1 != cache_msg.type) {
				log_e("The received message does not match the cache message");
				return false;
			}

			log_d(3, "Send Cache Message");
			if(!(send_udp(cache_msg))) {
				log_e("Failed to send Cache Message");
				return false;
			}

			uloop_timeout_cancel(&timer_keepalive);
			uloop_timeout_set(&timer_keepalive, keepalive_interval * 1000);
			return false;
		}
		if(control_val.seq_num != controller_seqnum) {
			if(control_val.seq_num < controller_seqnum)
				log_e("The serial number of the message is expired");
			else
				log_e("The serial number of the message is invalid");
			return false;
		}

		switch(control_val.msg_type)
		{
			case MSGTYPE_CONFIGURATION_REQUEST:
			{
				u8* list = NULL;
				int listSize; 
				if(!(parse_conf_req(&complete_msg, control_val.msg_len, &list, &listSize))) {
					return false;
				}

				protocol_msg responseMsg;
				init_protocol_msg(responseMsg);
				if(!(assemble_conf_resp(&responseMsg, list, listSize))) {
					log_e("Failed to assemble Configuration Response");
					return false;
				}
				log_i("Send Configuration Response");
				if(!(send_udp(responseMsg))) {
					log_e("Failed to send Configuration Response");
					return false;
				}
				free_object(list);

				free_protocol_msg(cache_msg);
				init_protocol_msg_size(cache_msg, responseMsg.offset, return false;);
				store_msg(&cache_msg, &responseMsg);
				cache_msg.type = MSGTYPE_CONFIGURATION_RESPONSE; //easy to match request
				free_protocol_msg(responseMsg);

				controller_seqnum_inc();
				uloop_timeout_cancel(&timer_keepalive);
				uloop_timeout_set(&timer_keepalive, keepalive_interval * 1000);
				break;
			}
			case MSGTYPE_CONFIGURATION_UPDATE_REQUEST:
			{
				if(!(parse_conf_update_req(&complete_msg, control_val.msg_len))) {
					return false;
				}

				protocol_msg responseMsg;
				init_protocol_msg(responseMsg);
				if(!(assemble_conf_update_resp(&responseMsg))) {
					log_e("Failed to assemble Configuration Update Response");
					return false;
				}
				log_i("Send Configuration Update Response");
				if(!(send_udp(responseMsg))) {
					log_e("Failed to send Configuration Update Response");
					return false;
				}

				free_protocol_msg(cache_msg);
				init_protocol_msg_size(cache_msg, responseMsg.offset, return false;);
				store_msg(&cache_msg, &responseMsg);
				cache_msg.type = MSGTYPE_CONFIGURATION_UPDATE_RESPONSE; //easy to match request
				free_protocol_msg(responseMsg);

				controller_seqnum_inc();
				uloop_timeout_cancel(&timer_keepalive);
				uloop_timeout_set(&timer_keepalive, keepalive_interval * 1000);
				break;
			}
			case MSGTYPE_SYSTEM_REQUEST:
			{
				if(!(parse_system_req(&complete_msg, control_val.msg_len))) {
					return false;
				}

				protocol_msg responseMsg;
				init_protocol_msg(responseMsg);
				if(!(assemble_system_resp(&responseMsg))) {
					log_e("Failed to assemble System Response");
					return false;
				}
				log_i("Send System Response");
				if(!(send_udp(responseMsg))) {
					log_e("Failed to send System Response");
					return false;
				}

				free_protocol_msg(cache_msg);
				init_protocol_msg_size(cache_msg, responseMsg.offset, return false;);
				store_msg(&cache_msg, &responseMsg);
				cache_msg.type = MSGTYPE_SYSTEM_RESPONSE; //easy to match request
				free_protocol_msg(responseMsg);

				controller_seqnum_inc();
				uloop_timeout_cancel(&timer_keepalive);
				uloop_timeout_set(&timer_keepalive, keepalive_interval * 1000);
				break;
			}
			case MSGTYPE_UNREGISTER_REQUEST:
			{
				log_i("Accept Unregister Request");
				uloop_end(); //goto AP_DOWN

				protocol_msg responseMsg;
				init_protocol_msg(responseMsg);
				if(!(assemble_unregister_resp(&responseMsg))) {
					log_e("Failed to assemble Unregister Response");
					return false;
				}
				log_i("Send Unregister Response");
				if(!(send_udp(responseMsg))) {
					log_e("Failed to send Unregister Response");
					return false;
				}
				free_protocol_msg(cache_msg);
				uloop_timeout_cancel(&timer_keepalive);
				break;
			}
			default:
				return false;
		}


	} else {
		if(control_val.seq_num != ap_seqnum) {
			if(control_val.seq_num < ap_seqnum)
				log_e("The serial number of the message is expired");
			else
				log_e("The serial number of the message is invalid");
			return false;
		}
		if(retransmit_msg.type == 0 || control_val.msg_type != retransmit_msg.type + 1) {
			log_e("The received message does not match the send message");
			return false;
		}
		switch(control_val.msg_type)
		{
			case MSGTYPE_KEEPALIVE_RESPONSE:
				if(!(parse_keepalive_resp())) {
					return false;
				}
				uloop_timeout_cancel(&timer_retransmit);
				free_protocol_msg(retransmit_msg);
				ap_seqnum_inc();
				break;
			default:
				return false;
		}
	}

	return true;
}

static void ap_cb(struct uloop_fd *event_fd, unsigned int events)
{
	int sockfd = event_fd->fd;
	if(sockfd != ap_socket) {
		log_e("Received an event from unknown source");
	}
	msg_handle_in_run();
}

ap_state enter_run() 
{
	log_i("");	
	log_i("######### Run State #########");

	free_protocol_msg(cache_msg);
	free_protocol_msg(retransmit_msg);


	uloop_init();

	timer_retransmit.cb = retransmit_handler;
	timer_keepalive.cb = keepalive_handler;
	uloop_timeout_set(&timer_keepalive, 100); //send a keep alive req soon

	fd_socket.fd = ap_socket;
	fd_socket.cb = ap_cb;
	uloop_fd_add(&fd_socket, ULOOP_READ);

	uloop_run();

	uloop_done();

	return ENTER_DOWN;
}