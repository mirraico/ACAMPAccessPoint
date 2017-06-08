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


static int discovery_cnt;
static int max_discovery = 3;
static int discovery_interval = 10;

#define MAX_WAIT_CONTROLLER 5
static int controller_num;
static controller_val controllers[MAX_WAIT_CONTROLLER];


bool evaluate_controller()
{
	int i;
	/* now the strategy is choose the first */
	create_object(controller_name, (strlen(controllers[0].name) + 1), return false;);
	copy_memory(controller_name, controllers[0].name, strlen(controllers[0].name));
	controller_name[strlen(controllers[0].name)] = '\0';

	create_object(controller_desc, (strlen(controllers[0].descriptor) + 1), return false;);
	copy_memory(controller_desc, controllers[0].descriptor, strlen(controllers[0].descriptor));
	controller_desc[strlen(controllers[0].descriptor)] = '\0';

	controller_ip = controllers[0].ip_addr; 
	for(i = 0; i < 6; i++) {
		controller_mac[i] = controllers[0].mac_addr[i];
	}
	return true;
}

bool assemble_discovery_req(protocol_msg *msg_p)
{
	if(msg_p == NULL) return false;
	
	protocol_msg *msgelems;
	int msgelem_cnt = 0;
	create_protocol_arr(msgelems, msgelem_cnt, return false;);
	
	return assemble_msg(msg_p, 
				 ap_apid,
				 ap_seqnum,
				 MSGTYPE_DISCOVERY_REQUEST,
				 msgelems,
				 msgelem_cnt
	);
}

bool parse_discovery_resp(char *msg, 
					   int len,
					   int current_seqnum,
					   controller_val *controller_p) 
{
	header_val control_val;
	protocol_msg complete_msg;

	u16 elem_flag = 0;
	
	if(msg == NULL || controller_p == NULL) 
		return false;
	
	log_i("Parse Discovery Response");
	
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
	if(control_val.msg_type != MSGTYPE_DISCOVERY_RESPONSE) {
		log_e("Message is not Discovery Response as Expected");
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
			case MSGELEMTYPE_CONTROLLER_NAME:
				if(elem_flag & 0x01) {
					parse_repeated_msgelem(&complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(parse_controller_name(&complete_msg, len, &(controller_p->name))))
					return false;
				elem_flag |= 0x01;
				break;
			case MSGELEMTYPE_CONTROLLER_DESCRIPTOR:
				if(elem_flag & 0x02) {
					parse_repeated_msgelem(&complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(parse_controller_desc(&complete_msg, len, &(controller_p->descriptor))))
					return false;
				elem_flag |= 0x02;
				break;
			case MSGELEMTYPE_CONTROLLER_IP_ADDR:
				if(elem_flag & 0x04) {
					parse_repeated_msgelem(&complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(parse_controller_ip(&complete_msg, len, &(controller_p->ip_addr))))
					return false;
				elem_flag |= 0x04;
				break;
			case MSGELEMTYPE_CONTROLLER_MAC_ADDR:
				if(elem_flag & 0x08) {
					parse_repeated_msgelem(&complete_msg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(parse_controller_mac(&complete_msg, len, controller_p->mac_addr)))
					return false;
				elem_flag |= 0x08;
				break;
			
			default:
				parse_unrecognized_msgelem(&complete_msg, len);
				log_e("Unrecognized Message Element");
				// return APErrorRaise(AP_ERROR_INVALID_FORMAT,
				// 	"parse_discovery_resp()");
		}
	}

	if(elem_flag != 0x0F) {
		log_e("Incomplete Message Element in Discovery Response");
		return false;
	}
	return true;
}

bool receive_discovery_resp() {
	int i;
	char buf[BUFFER_SIZE];
	struct sockaddr_in addr;
	controller_val *controller_p = &controllers[controller_num];
	int read_bytes;
	u32 recv_addr;

	controller_p->name = NULL;
	controller_p->descriptor = NULL;
	controller_p->ip_addr = 0;
	for(i = 0; i < 6; i++) {
		controller_p->mac_addr[i] = 0;
	}

	/* receive the datagram */
	if(!(recv_udp_br(buf,
					 BUFFER_SIZE - 1,
					 &addr,
					 &read_bytes))) {
		log_e("Receive Discovery Response failed");
		return false;
	}

	recv_addr = ntohl(addr.sin_addr.s_addr);
	log_i("Receive Discovery Response from %u.%u.%u.%u", (u8)(recv_addr >> 24), (u8)(recv_addr >> 16),\
	  (u8)(recv_addr >> 8),  (u8)(recv_addr >> 0));
	
	/* check if it is a valid Discovery Response */
	if(!(parse_discovery_resp(buf, read_bytes, ap_seqnum, controller_p))) {
		return false;
	}

	/* the address declared by controller is fake */
	if(ntohl(addr.sin_addr.s_addr) != controller_p->ip_addr) {
		log_e("The source address and the address carried in the packet do not match");
		return false;
	}
	
	log_i("Accept valid Discovery Response from %u.%u.%u.%u", (u8)((controller_p->ip_addr) >> 24), (u8)((controller_p->ip_addr) >> 16),\
	  (u8)((controller_p->ip_addr) >> 8),  (u8)((controller_p->ip_addr) >> 0));
	
	return true;
}

bool read_discovery_resp() 
{

	struct timeval timeout, new_timeout, before, after, delta;
	
	timeout.tv_sec = new_timeout.tv_sec = discovery_interval;
	timeout.tv_usec = new_timeout.tv_usec = 0;

	gettimeofday(&before, NULL); // set current time
	
	while(1) 
	{
		bool success = false;

		if(time_poll_read(ap_socket_br, &new_timeout)) 
		{
			success = true;
		}

		if(!success) goto ap_time_over;

		if((receive_discovery_resp())) {
			controller_num++;
		}
		/* compute time and go on */
		gettimeofday(&after, NULL);
		tv_subtract(&delta, &after, &before);
		if(tv_subtract(&new_timeout, &timeout, &delta) == 1) { 
			/* time is over */
			goto ap_time_over;
		}

	}
	ap_time_over:
		log_d(3, "Timer expired during read Discovery Response");	

	if(controller_num == 0) {
		log_i("There is no response or valid Controller");
		return false;
	} 
	
	log_i("There is(are) %d controller(s) available", controller_num);
	return true;
}

ap_state enter_discovery() 
{
	log_i("");	
	log_i("######### Discovery State #########");

	/*reset discovery count*/
	discovery_cnt = 0;

	if(!init_broadcast()) {
		log_e("Init broadcast socket failed");
		return ENTER_DOWN;
	}

	while(1)
	{
		controller_num = 0;

		if(discovery_cnt == max_discovery) {
			log_i("No Discovery Responses for 3 times");
			ap_seqnum_inc();
			return ENTER_DOWN;
		}
		
		protocol_msg send_msg;
		init_protocol_msg(send_msg);
		log_d(3, "Assemble Discovery Request");
		if(!(assemble_discovery_req(&send_msg))) {
			log_e("Failed to assemble Discovery Request");
			return ENTER_DOWN;
		}
		log_i("Send Discovery Request");
		if(!(send_udp_br(send_msg))) {
			log_e("Failed to send Discovery Request");
			return ENTER_DOWN;
		}
		free_protocol_msg(send_msg);

		discovery_cnt++;
		log_d(3, "The number of discovery operations = %d", discovery_cnt);

		/* wait for Responses */
		if(!(read_discovery_resp())) {
			continue; // no available controller
		}

		//choose one controller
		if(!(evaluate_controller())) {
			continue;
		}
		log_i("Picks a Controller from %u.%u.%u.%u", (u8)(controller_ip >> 24), (u8)(controller_ip >> 16),\
			(u8)(controller_ip >> 8),  (u8)(controller_ip >> 0));
		break;
	}

	ap_seqnum_inc();
	log_i("The discovery state is finished");
	return ENTER_REGISTER;
}
