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
	create_object(gControllerName, (strlen(controllers[0].name) + 1), return false;);
	copy_memory(gControllerName, controllers[0].name, strlen(controllers[0].name));
	gControllerName[strlen(controllers[0].name)] = '\0';

	create_object(gControllerDescriptor, (strlen(controllers[0].descriptor) + 1), return false;);
	copy_memory(gControllerDescriptor, controllers[0].descriptor, strlen(controllers[0].descriptor));
	gControllerDescriptor[strlen(controllers[0].descriptor)] = '\0';

	gControllerIPAddr = controllers[0].IPAddr; 
	for(i = 0; i < 6; i++) {
		gControllerMACAddr[i] = controllers[0].MACAddr[i];
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
				 APGetAPID(),
				 APGetSeqNum(),
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
	
	APLog("Parse Discovery Response");
	
	completeMsg.msg = msg;
	completeMsg.offset = 0;
	
	if(!(APParseControlHeader(&completeMsg, &controlVal))) {
		APErrorLog("Failed to parse header");
		return false;
	}
	
	/* not as expected */
	if(controlVal.version != CURRENT_VERSION || controlVal.type != TYPE_CONTROL) {
		APErrorLog("ACAMP version or type is not Expected");
		return false;
	}
	if(controlVal.seqNum != currentSeqNum) {
		APErrorLog("Sequence Number of Response doesn't match Request");
		return false;
	}
	if(controlVal.msgType != MSGTYPE_DISCOVERY_RESPONSE) {
		APErrorLog("Message is not Discovery Response as Expected");
		return false;
	}

	/* parse message elements */
	while(completeMsg.offset < controlVal.msgLen) 
	{
		u16 type = 0;
		u16 len = 0;

		APParseFormatMsgElem(&completeMsg, &type, &len);
		// APDebugLog(3, "Parsing Message Element: %u, len: %u", type, len);
		
		switch(type) 
		{
			case MSGELEMTYPE_CONTROLLER_NAME:
				if(elemFlag & 0x01) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseControllerName(&completeMsg, len, &(controllerPtr->name))))
					return false;
				elemFlag |= 0x01;
				break;
			case MSGELEMTYPE_CONTROLLER_DESCRIPTOR:
				if(elemFlag & 0x02) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseControllerDescriptor(&completeMsg, len, &(controllerPtr->descriptor))))
					return false;
				elemFlag |= 0x02;
				break;
			case MSGELEMTYPE_CONTROLLER_IP_ADDR:
				if(elemFlag & 0x04) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseControllerIPAddr(&completeMsg, len, &(controllerPtr->IPAddr))))
					return false;
				elemFlag |= 0x04;
				break;
			case MSGELEMTYPE_CONTROLLER_MAC_ADDR:
				if(elemFlag & 0x08) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseControllerMACAddr(&completeMsg, len, controllerPtr->MACAddr)))
					return false;
				elemFlag |= 0x08;
				break;
			
			default:
				APParseUnrecognizedMsgElem(&completeMsg, len);
				APErrorLog("Unrecognized Message Element");
				// return APErrorRaise(AP_ERROR_INVALID_FORMAT,
				// 	"APParseDiscoveryResponse()");
		}
	}

	if(elemFlag != 0x0F) {
		APErrorLog("Incomplete Message Element in Discovery Response");
		return false;
	}
	return true;
}

bool APReceiveDiscoveryResponse() {
	int i;
	char buf[BUFFER_SIZE];
	APNetworkAddress addr;
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
	if(!(APNetworkReceiveFromBroad(buf,
					 BUFFER_SIZE - 1,
					 &addr,
					 &readBytes))) {
		APErrorLog("Receive Discovery Response failed");
		return false;
	}

	recvAddr = ntohl(addr.sin_addr.s_addr);
	APLog("Receive Discovery Response from %u.%u.%u.%u", (u8)(recvAddr >> 24), (u8)(recvAddr >> 16),\
	  (u8)(recvAddr >> 8),  (u8)(recvAddr >> 0));
	
	/* check if it is a valid Discovery Response */
	if(!(APParseDiscoveryResponse(buf, readBytes, APGetSeqNum(), controllerPtr))) {
		return false;
	}

	/* the address declared by controller is fake */
	if(ntohl(addr.sin_addr.s_addr) != controllerPtr->IPAddr) {
		APErrorLog("The source address and the address carried in the packet do not match");
		return false;
	}
	
	APLog("Accept valid Discovery Response from %u.%u.%u.%u", (u8)((controllerPtr->IPAddr) >> 24), (u8)((controllerPtr->IPAddr) >> 16),\
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

		if(APNetworkTimedPollRead(gSocketBroad, &new_timeout)) 
		{
			success = true;
		}

		if(!success) goto ap_time_over;

		if((APReceiveDiscoveryResponse())) {
			foundControllerCount++;
		}
		/* compute time and go on */
		gettimeofday(&after, NULL);
		timeval_subtract(&delta, &after, &before);
		if(timeval_subtract(&new_timeout, &timeout, &delta) == 1) { 
			/* time is over */
			goto ap_time_over;
		}

	}
	ap_time_over:
		APDebugLog(3, "Timer expired during read Discovery Response");	

	if(foundControllerCount == 0) {
		APLog("There is no response or valid Controller");
		return false;
	} 
	
	APLog("There is(are) %d controller(s) available", foundControllerCount);
	return true;
}

APStateTransition APEnterDiscovery() 
{
	APLog("");	
	APLog("######### Discovery State #########");

	/*reset discovery count*/
	gDiscoveryCount = 0;

	if(!APNetworkInitBroadcast()) {
		APErrorLog("Init broadcast socket failed");
		return AP_ENTER_DOWN;
	}

	while(1)
	{
		foundControllerCount = 0;

		if(gDiscoveryCount == gMaxDiscovery) {
			APLog("No Discovery Responses for 3 times");
			APSeqNumIncrement();
			return AP_ENTER_DOWN;
		}
		
		APProtocolMessage sendMsg;
		AP_INIT_PROTOCOL(sendMsg);
		APDebugLog(3, "Assemble Discovery Request");
		if(!(APAssembleDiscoveryRequest(&sendMsg))) {
			APErrorLog("Failed to assemble Discovery Request");
			return AP_ENTER_DOWN;
		}
		APLog("Send Discovery Request");
		if(!(APNetworkSendToBroad(sendMsg))) {
			APErrorLog("Failed to send Discovery Request");
			return AP_ENTER_DOWN;
		}
		AP_FREE_PROTOCOL_MESSAGE(sendMsg);

		gDiscoveryCount++;
		APDebugLog(3, "The number of discovery operations = %d", gDiscoveryCount);

		/* wait for Responses */
		if(!(APReadDiscoveryResponse())) {
			continue; // no available controller
		}

		//choose one controller
		if(!(APEvaluateController())) {
			continue;
		}
		APLog("Picks a Controller from %u.%u.%u.%u", (u8)(gControllerIPAddr >> 24), (u8)(gControllerIPAddr >> 16),\
			(u8)(gControllerIPAddr >> 8),  (u8)(gControllerIPAddr >> 0));
		break;
	}

	APSeqNumIncrement();
	APLog("The discovery state is finished");
	return AP_ENTER_REGISTER;
}
