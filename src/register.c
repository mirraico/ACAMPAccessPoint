#include "common.h"
#include "error.h"
#include "log.h"
#include "ap.h"
#include "network.h"

typedef struct {
	char* name;
	char* descriptor;
	u32 ip_addr; 
	u8 mac_addr[6];
} controller_val;

int rejected;

static int register_cnt;
static int max_register;
static int register_interval;

static u16 recv_apid;
static u8 recv_register_service;
static u32 recv_controller_seqnum;

bool assemble_register_req(protocol_msg *msg_p)
{
	int k = -1;
	if(msg_p == NULL) return false;
	
	protocol_msg *msgelems;
	int msgelem_cnt = 6;
	create_protocol_arr(msgelems, msgelem_cnt, return false;);

	if(
	   (!(assemble_register_service(&(msgelems[++k])))) ||
	   (!(assemble_ap_name(&(msgelems[++k])))) ||
	   (!(assemble_ap_desc(&(msgelems[++k])))) ||
	   (!(assemble_ap_ip(&(msgelems[++k])))) ||
	   (!(assemble_ap_mac(&(msgelems[++k])))) ||
	   (!(assemble_ap_discovery_type(&(msgelems[++k]))))
	   )
	{
		int i;
		for(i = 0; i <= k; i++) { free_protocol_msg(msgelems[i]);}
		free_object(msgelems);
		return false;
	}
	
	return assemble_msg(msg_p, 
				 ap_apid,
				 ap_seqnum,
				 MSGTYPE_REGISTER_REQUEST,
				 msgelems,
				 msgelem_cnt
	);
}

bool parse_register_resp(char *msg, 
					   int len,
					   int current_seqnum) 
{
	
	u16 result, reason;
	header_val control_val;
	protocol_msg complete_msg;
	controller_val recv_controller_info;

	u16 success_flag = 0;
	u16 failure_flag = 0;

	rejected = false;
	
	if(msg == NULL) 
		return false;
	
	log_i("Parse Register Response");
	
	complete_msg.msg = msg;
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
	if(control_val.seq_num != current_seqnum) {
		log_e("Sequence Number of Response doesn't match Request");
		return false;
	}
	if(control_val.msg_type != MSGTYPE_REGISTER_RESPONSE) {
		log_e("Message is not Register Response as Expected");
		return false;
	}

	/* parse message elements */
	while(complete_msg.offset < control_val.msg_len) 
	{
		u16 type = 0;
		u16 len = 0;

		parse_msgelem(&complete_msg, &type, &len);
		// log_d(3, "Parsing Message Element: %u, len: %u", type, len);
		
		switch(type) 
		{
			case MSGELEMTYPE_RESULT_CODE:
				if((success_flag & 0x01) || failure_flag & 0x01) {
					parse_repeated_msgelem(&complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(parse_res_code(&complete_msg, len, &result)))
					return false;
				if(
					result != RESULT_SUCCESS &&
					result != RESULT_FAILURE 
					//result != RESULT_UNRECOGNIZED_ELEM
				) {
					log_e("Unrecognized Result Code");
					/* needn't' return but go on */
					break;
				}
				success_flag |= 0x01; failure_flag |= 0x01;
				break;
			case MSGELEMTYPE_REASON_CODE:
				if(failure_flag & 0x02) {
					parse_repeated_msgelem(&complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(parse_reason_code(&complete_msg, len, &reason)))
					return false;
				/* consider unknown reason */
				// if(
				// 	reason != REASON_INVALID_VERSION &&
				// 	reason != REASON_REPEATED_REGISTER &&
				// 	reason != REASON_INSUFFICIENT_RESOURCE
				// ) {
				// 	log_e("Unrecognized Reason Code");
				// 	/* needn't' return but go on */
				// 	break;
				// }
				failure_flag |= 0x02;
				break;
			case MSGELEMTYPE_REGISTERED_SERVICE:
				if(success_flag & 0x02) {
					parse_repeated_msgelem(&complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(parse_register_service(&complete_msg, len, &recv_register_service)))
					return false;
				if(
					recv_register_service != REGISTERED_SERVICE_CONF_STA
				) {
					log_e("Unrecognized Registered Service");
					/* needn't' return but go on */
					break;
				}
				success_flag |= 0x02;
				break;
			case MSGELEMTYPE_ASSIGNED_APID:
				if(success_flag & 0x04) {
					parse_repeated_msgelem(&complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(parse_assigned_apid(&complete_msg, len, &recv_apid)))
					return false;
				success_flag |= 0x04;
				break;
			case MSGELEMTYPE_CONTROLLER_NAME:
				if(success_flag & 0x08) {
					parse_repeated_msgelem(&complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(parse_controller_name(&complete_msg, len, &recv_controller_info.name)))
					return false;
				success_flag |= 0x08;
				break;
			case MSGELEMTYPE_CONTROLLER_DESCRIPTOR:
				if(success_flag & 0x10) {
					parse_repeated_msgelem(&complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(parse_controller_desc(&complete_msg, len, &recv_controller_info.descriptor)))
					return false;
				success_flag |= 0x10;
				break;
			case MSGELEMTYPE_CONTROLLER_IP_ADDR:
				if(success_flag & 0x20) {
					parse_repeated_msgelem(&complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(parse_controller_ip(&complete_msg, len, &recv_controller_info.ip_addr)))
					return false;
				success_flag |= 0x20;
				break;
			case MSGELEMTYPE_CONTROLLER_MAC_ADDR:
				if(success_flag & 0x40) {
					parse_repeated_msgelem(&complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(parse_controller_mac(&complete_msg, len, recv_controller_info.mac_addr)))
					return false;
				success_flag |= 0x40;
				break;
			case MSGELEMTYPE_CONTROLLER_NEXTSEQ:
				if(success_flag & 0x80) {
					parse_repeated_msgelem(&complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(parse_controller_nextseq(&complete_msg, len, &recv_controller_seqnum)))
					return false;
				success_flag |= 0x80;
				break;
			
			default:
				parse_unrecognized_msgelem(&complete_msg, len);
				log_e("Unrecognized Message Element");
				// return APErrorRaise(AP_ERROR_INVALID_FORMAT,
				// 	"parse_register_resp()");
		}
	}

	if(success_flag != 0xFF && failure_flag != 0x03) { //incomplete message
		log_e("Incomplete Message Element in Register Response");
		return false;
	}

	/* reject */
	if(result == RESULT_FAILURE)
	{
		/* If rejected, The message should contain only result & reason element */
		if(failure_flag != 0x03 || success_flag != 0x01) {
			log_e("The Message carrying some wrong information");
			return false;
		}
		rejected = true;
		switch(reason) {
			case REASON_INVALID_VERSION:
				log_i("Controller rejected the Register Request, the reason is protocol version does not match");
				return false;
			// case REASON_REPEATED_REGISTER:
			// 	log_i("Controller rejected the Register Request, the reason is duplicated service request");
			// 	return false;
			case REASON_INSUFFICIENT_RESOURCE:
				log_i("Controller rejected the Register Request, because there is no enough resources");
				return false;
			default:
				log_i("Controller rejected the Register Request, for unknown reasons");
				return false;
		}
		return false;
	}

	// if(result == RESULT_UNRECOGNIZED_ELEM) { //TODO: RESULT_UNRECOGNIZED_ELEM
	// 	log_e("This Result is not currently being processed");
	//     return false;
	// }

	/* accept */
	/* If accepted, The message should contain only some specified element */
	if(success_flag != 0xFF || failure_flag != 0x01) {
		log_e("The Message carrying some wrong information");
		return false;
	}

	if(ap_register_service != recv_register_service) {
		log_e("The service is different from the requested one");
		return false;
	}

	if(control_val.apid != recv_apid) {
		log_e("The apid in message is different from the one in message header");
	}

	if(controller_name == NULL || strcmp(controller_name, recv_controller_info.name) != 0) {
		if(ap_discovery_type == DISCOVERY_TPYE_DISCOVERY) log_e("The Controller name has changed");
		free_object(controller_name);
		create_object(controller_name, (strlen(recv_controller_info.name) + 1), return false;);
		copy_memory(controller_name, recv_controller_info.name, strlen(recv_controller_info.name));
		controller_name[strlen(recv_controller_info.name)] = '\0';
	}

	if(controller_desc == NULL || strcmp(controller_desc, recv_controller_info.descriptor) != 0) {
		if(ap_discovery_type == DISCOVERY_TPYE_DISCOVERY) log_e("The Controller descriptor has changed");
		free_object(controller_desc);
		create_object(controller_desc, (strlen(recv_controller_info.descriptor) + 1), return false;);
		copy_memory(controller_desc, recv_controller_info.descriptor, strlen(recv_controller_info.descriptor));
		controller_desc[strlen(recv_controller_info.descriptor)] = '\0';
	}

	if(controller_ip != recv_controller_info.ip_addr) {
		log_e("The Controller IP Addr has changed");
		return false;
	}

	if(
		controller_mac[0] != recv_controller_info.mac_addr[0] ||
		controller_mac[1] != recv_controller_info.mac_addr[1] ||
		controller_mac[2] != recv_controller_info.mac_addr[2] ||
		controller_mac[3] != recv_controller_info.mac_addr[3] ||
		controller_mac[4] != recv_controller_info.mac_addr[4] ||
		controller_mac[5] != recv_controller_info.mac_addr[5]
	) 
	{
		int i;
		if(ap_discovery_type == DISCOVERY_TPYE_DISCOVERY) log_e("The Controller MAC Addr has changed");
		for(i = 0; i < 6; i++) {
			controller_mac[i] = recv_controller_info.mac_addr[i];
		}
	}

	return true;
}

bool receive_register_resp() 
{
	char buf[BUFFER_SIZE];
	struct sockaddr_in addr;
	int read_bytes;
	u32 recv_addr;

	recv_apid = recv_register_service = 0;
	
	/* receive the datagram */
	if(!(recv_udp(buf,
					 BUFFER_SIZE - 1,
					 &addr,
					 &read_bytes))) {
		log_e("Receive Register Response failed");
		return false;
	}

	recv_addr = ntohl(addr.sin_addr.s_addr);
	/* verify the source of the message */
	if(recv_addr != controller_ip) {
		log_e("Message from the illegal source address");
		return false;
	}
	
	/* check if it is a valid Register Response */
	if(!(parse_register_resp(buf, read_bytes, ap_seqnum))) {
		return false;
	}

	if(rejected) return false;
	log_d(3, "Accept Register Response");
	
	return true;
}

bool read_register_resp() 
{

	struct timeval timeout, new_timeout, before, after, delta;
	
	timeout.tv_sec = new_timeout.tv_sec = register_interval;
	timeout.tv_usec = new_timeout.tv_usec = 0;

	gettimeofday(&before, NULL); // set current time
	

	while(1) 
	{
		bool success = false;

		if(time_poll_read(ap_socket, &new_timeout)) 
		{
			success = true;
		}

		if(!success) goto ap_time_over;

		
		if((receive_register_resp())) {
			return true;
		}

		/* if rejected, break */
		if(rejected) {
			return false;
		}

		/* compute time and go on */
		gettimeofday(&after, NULL);
		tv_subtract(&delta, &after, &before);
		if(tv_subtract(&new_timeout, &timeout, &delta) == 1) { 
			/* time is over (including receive & pause) */
			goto ap_time_over;
		}
	}
	ap_time_over:
		log_d(3, "Timer expired during read register response");
	
	log_i("There is no valid Response");
	return false;
}

ap_state enter_register() 
{
	log_i("");	
	log_i("######### Register State #########");

	register_cnt = 0;
	max_register = max_retransmit;
	register_interval = retransmit_interval;

	if(!init_controller_addr(controller_ip)) {
		log_e("Init singlecast socket failed");
		return ENTER_DOWN;
	}

	while(1)
	{
		if(register_cnt == max_register) {
			log_i("No Register Responses for 3 times");
			ap_seqnum_inc();
			return ENTER_DOWN;
		}
		
		protocol_msg send_msg;
		init_protocol_msg(send_msg);
		log_d(3, "Assemble Register Request");
		if(!(assemble_register_req(&send_msg))) {
			log_e("Failed to assemble Register Request");
			return ENTER_DOWN;
		}
		log_i("Send Register Request");
		if(!(send_udp(send_msg))) {
			log_e("Failed to send Register Request");
			return ENTER_DOWN;
		}
		free_protocol_msg(send_msg);

		register_cnt++;
		log_d(3, "The number of REGISTER operations = %d", register_cnt);

		/* wait for Responses */
		if(!(read_register_resp())) {
			if(rejected) return ENTER_DOWN; //rejected, do not need to repeated request
			register_interval *= 2;
			if(register_interval > (keepalive_interval / 2)) {
				register_interval = keepalive_interval / 2;
			}
			log_d(5, "Adjust the register interval to %d sec", register_interval);
			continue; // no response or invalid response
		}

		//set apid
		log_i("Accept valid Register Response and assigned APID is %d", recv_apid);
		log_i("Controller Next Seq Num is %d", recv_controller_seqnum);
		ap_apid = recv_apid;
		controller_seqnum = recv_controller_seqnum;
		log_i("Registered service successfully");
		break;
	}

	ap_seqnum_inc();
	log_i("The register state is finished");
	return ENTER_RUN;
}
