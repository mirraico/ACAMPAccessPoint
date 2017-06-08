#include "common.h"
#include "error.h"
#include "log.h"
#include "ap.h"
#include "network.h"

typedef struct {
	char* name;
	char* descriptor;
	u32 IPAddr; 
	u8 MACAddr[6];
} controllerVal;


int gDiscoveryCount;
int gMaxDiscovery = 3;
int gDiscoveryInterval = 10;

#define MAX_WAIT_CONTROLLER 5
int foundControllerCount;
controllerVal controllers[MAX_WAIT_CONTROLLER];


bool APEvaluateController()
{
	int i;
	/* now the strategy is choose the first */
	create_object(controller_name, (strlen(controllers[0].name) + 1), return false;);
	copy_memory(controller_name, controllers[0].name, strlen(controllers[0].name));
	controller_name[strlen(controllers[0].name)] = '\0';

	create_object(controller_desc, (strlen(controllers[0].descriptor) + 1), return false;);
	copy_memory(controller_desc, controllers[0].descriptor, strlen(controllers[0].descriptor));
	controller_desc[strlen(controllers[0].descriptor)] = '\0';

	controller_ip = controllers[0].IPAddr; 
	for(i = 0; i < 6; i++) {
		controller_mac[i] = controllers[0].MACAddr[i];
	}
	return true;
}

bool APAssembleDiscoveryRequest(APProtocolMessage *messagesPtr)
{
	if(messagesPtr == NULL) return false;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return false;);
	
	return APAssembleControlMessage(messagesPtr, 
				 ap_apid,
				 ap_seqnum,
				 MSGTYPE_DISCOVERY_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

bool APParseDiscoveryResponse(char *msg, 
					   int len,
					   int currentSeqNum,
					   controllerVal *controllerPtr) 
{
	APHeaderVal controlVal;
	APProtocolMessage completeMsg;

	u16 elemFlag = 0;
	
	if(msg == NULL || controllerPtr == NULL) 
		return false;
	
	log("Parse Discovery Response");
	
	completeMsg.msg = msg;
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
	if(controlVal.seqNum != currentSeqNum) {
		log_e("Sequence Number of Response doesn't match Request");
		return false;
	}
	if(controlVal.msgType != MSGTYPE_DISCOVERY_RESPONSE) {
		log_e("Message is not Discovery Response as Expected");
		return false;
	}

	/* parse message elements */
	while(completeMsg.offset < controlVal.msgLen) 
	{
		u16 type = 0;
		u16 len = 0;

		APParseFormatMsgElem(&completeMsg, &type, &len);
		// log_d(3, "Parsing Message Element: %u, len: %u", type, len);
		
		switch(type) 
		{
			case MSGELEMTYPE_CONTROLLER_NAME:
				if(elemFlag & 0x01) {
					APParseRepeatedMsgElem(&completeMsg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(APParseControllerName(&completeMsg, len, &(controllerPtr->name))))
					return false;
				elemFlag |= 0x01;
				break;
			case MSGELEMTYPE_controller_descCRIPTOR:
				if(elemFlag & 0x02) {
					APParseRepeatedMsgElem(&completeMsg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(APParseControllerDescriptor(&completeMsg, len, &(controllerPtr->descriptor))))
					return false;
				elemFlag |= 0x02;
				break;
			case MSGELEMTYPE_CONTROLLER_IP_ADDR:
				if(elemFlag & 0x04) {
					APParseRepeatedMsgElem(&completeMsg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(APParseControllerIPAddr(&completeMsg, len, &(controllerPtr->IPAddr))))
					return false;
				elemFlag |= 0x04;
				break;
			case MSGELEMTYPE_CONTROLLER_MAC_ADDR:
				if(elemFlag & 0x08) {
					APParseRepeatedMsgElem(&completeMsg, len);
					log_e("Repeated Message Element");
					break;
				}
				if(!(APParseControllerMACAddr(&completeMsg, len, controllerPtr->MACAddr)))
					return false;
				elemFlag |= 0x08;
				break;
			
			default:
				APParseUnrecognizedMsgElem(&completeMsg, len);
				log_e("Unrecognized Message Element");
				// return APErrorRaise(AP_ERROR_INVALID_FORMAT,
				// 	"APParseDiscoveryResponse()");
		}
	}

	if(elemFlag != 0x0F) {
		log_e("Incomplete Message Element in Discovery Response");
		return false;
	}
	return true;
}

bool APReceiveDiscoveryResponse() {
	int i;
	char buf[BUFFER_SIZE];
	struct sockaddr_in addr;
	controllerVal *controllerPtr = &controllers[foundControllerCount];
	int readBytes;
	u32 recvAddr;

	controllerPtr->name = NULL;
	controllerPtr->descriptor = NULL;
	controllerPtr->IPAddr = 0;
	for(i = 0; i < 6; i++) {
		controllerPtr->MACAddr[i] = 0;
	}

	/* receive the datagram */
	if(!(recv_udp_br(buf,
					 BUFFER_SIZE - 1,
					 &addr,
					 &readBytes))) {
		log_e("Receive Discovery Response failed");
		return false;
	}

	recvAddr = ntohl(addr.sin_addr.s_addr);
	log("Receive Discovery Response from %u.%u.%u.%u", (u8)(recvAddr >> 24), (u8)(recvAddr >> 16),\
	  (u8)(recvAddr >> 8),  (u8)(recvAddr >> 0));
	
	/* check if it is a valid Discovery Response */
	if(!(APParseDiscoveryResponse(buf, readBytes, ap_seqnum, controllerPtr))) {
		return false;
	}

	/* the address declared by controller is fake */
	if(ntohl(addr.sin_addr.s_addr) != controllerPtr->IPAddr) {
		log_e("The source address and the address carried in the packet do not match");
		return false;
	}
	
	log("Accept valid Discovery Response from %u.%u.%u.%u", (u8)((controllerPtr->IPAddr) >> 24), (u8)((controllerPtr->IPAddr) >> 16),\
	  (u8)((controllerPtr->IPAddr) >> 8),  (u8)((controllerPtr->IPAddr) >> 0));
	
	return true;
}

bool APReadDiscoveryResponse() 
{

	struct timeval timeout, new_timeout, before, after, delta;
	
	timeout.tv_sec = new_timeout.tv_sec = gDiscoveryInterval;
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

		if((APReceiveDiscoveryResponse())) {
			foundControllerCount++;
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

	if(foundControllerCount == 0) {
		log("There is no response or valid Controller");
		return false;
	} 
	
	log("There is(are) %d controller(s) available", foundControllerCount);
	return true;
}

APStateTransition APEnterDiscovery() 
{
	log("");	
	log("######### Discovery State #########");

	/*reset discovery count*/
	gDiscoveryCount = 0;

	if(!init_broadcast()) {
		log_e("Init broadcast socket failed");
		return AP_ENTER_DOWN;
	}

	while(1)
	{
		foundControllerCount = 0;

		if(gDiscoveryCount == gMaxDiscovery) {
			log("No Discovery Responses for 3 times");
			ap_seqnum_inc();
			return AP_ENTER_DOWN;
		}
		
		APProtocolMessage sendMsg;
		AP_INIT_PROTOCOL(sendMsg);
		log_d(3, "Assemble Discovery Request");
		if(!(APAssembleDiscoveryRequest(&sendMsg))) {
			log_e("Failed to assemble Discovery Request");
			return AP_ENTER_DOWN;
		}
		log("Send Discovery Request");
		if(!(send_udp_br(sendMsg))) {
			log_e("Failed to send Discovery Request");
			return AP_ENTER_DOWN;
		}
		AP_FREE_PROTOCOL_MESSAGE(sendMsg);

		gDiscoveryCount++;
		log_d(3, "The number of discovery operations = %d", gDiscoveryCount);

		/* wait for Responses */
		if(!(APReadDiscoveryResponse())) {
			continue; // no available controller
		}

		//choose one controller
		if(!(APEvaluateController())) {
			continue;
		}
		log("Picks a Controller from %u.%u.%u.%u", (u8)(controller_ip >> 24), (u8)(controller_ip >> 16),\
			(u8)(controller_ip >> 8),  (u8)(controller_ip >> 0));
		break;
	}

	ap_seqnum_inc();
	log("The discovery state is finished");
	return AP_ENTER_REGISTER;
}
