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

int rejected;

int gRegisterCount;
int gMaxRegister;
int gRegisterInterval;

u16 recvAPID;
u8 recvRegisteredService;
u32 recvControllerSeqNum;

bool APAssembleRegisterRequest(APProtocolMessage *messagesPtr)
{
	int k = -1;
	if(messagesPtr == NULL) return false;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 6;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return false;);

	if(
	   (!(APAssembleRegisteredService(&(msgElems[++k])))) ||
	   (!(APAssembleAPName(&(msgElems[++k])))) ||
	   (!(APAssembleAPDescriptor(&(msgElems[++k])))) ||
	   (!(APAssembleAPIPAddr(&(msgElems[++k])))) ||
	   (!(APAssembleAPMACAddr(&(msgElems[++k])))) ||
	   (!(APAssembleDiscoveryType(&(msgElems[++k]))))
	   )
	{
		int i;
		for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		free_object(msgElems);
		return false;
	}
	
	return APAssembleControlMessage(messagesPtr, 
				 ap_apid,
				 ap_seqnum,
				 MSGTYPE_REGISTER_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

bool APParseRegisterResponse(char *msg, 
					   int len,
					   int currentSeqNum) 
{
	
	u16 result, reason;
	APHeaderVal controlVal;
	APProtocolMessage completeMsg;
	controllerVal recvControllerInfo;

	u16 successFlag = 0;
	u16 failureFlag = 0;

	rejected = false;
	
	if(msg == NULL) 
		return false;
	
	APLog("Parse Register Response");
	
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
	if(controlVal.msgType != MSGTYPE_REGISTER_RESPONSE) {
		APErrorLog("Message is not Register Response as Expected");
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
			case MSGELEMTYPE_RESULT_CODE:
				if((successFlag & 0x01) || failureFlag & 0x01) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseResultCode(&completeMsg, len, &result)))
					return false;
				if(
					result != RESULT_SUCCESS &&
					result != RESULT_FAILURE 
					//result != RESULT_UNRECOGNIZED_ELEM
				) {
					APErrorLog("Unrecognized Result Code");
					/* needn't' return but go on */
					break;
				}
				successFlag |= 0x01; failureFlag |= 0x01;
				break;
			case MSGELEMTYPE_REASON_CODE:
				if(failureFlag & 0x02) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseReasonCode(&completeMsg, len, &reason)))
					return false;
				/* consider unknown reason */
				// if(
				// 	reason != REASON_INVALID_VERSION &&
				// 	reason != REASON_REPEATED_REGISTER &&
				// 	reason != REASON_INSUFFICIENT_RESOURCE
				// ) {
				// 	APErrorLog("Unrecognized Reason Code");
				// 	/* needn't' return but go on */
				// 	break;
				// }
				failureFlag |= 0x02;
				break;
			case MSGELEMTYPE_REGISTERED_SERVICE:
				if(successFlag & 0x02) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseRegisteredService(&completeMsg, len, &recvRegisteredService)))
					return false;
				if(
					recvRegisteredService != REGISTERED_SERVICE_CONF_STA
				) {
					APErrorLog("Unrecognized Registered Service");
					/* needn't' return but go on */
					break;
				}
				successFlag |= 0x02;
				break;
			case MSGELEMTYPE_ASSIGNED_APID:
				if(successFlag & 0x04) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseAssignedAPID(&completeMsg, len, &recvAPID)))
					return false;
				successFlag |= 0x04;
				break;
			case MSGELEMTYPE_CONTROLLER_NAME:
				if(successFlag & 0x08) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseControllerName(&completeMsg, len, &recvControllerInfo.name)))
					return false;
				successFlag |= 0x08;
				break;
			case MSGELEMTYPE_CONTROLLER_DESCRIPTOR:
				if(successFlag & 0x10) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseControllerDescriptor(&completeMsg, len, &recvControllerInfo.descriptor)))
					return false;
				successFlag |= 0x10;
				break;
			case MSGELEMTYPE_CONTROLLER_IP_ADDR:
				if(successFlag & 0x20) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseControllerIPAddr(&completeMsg, len, &recvControllerInfo.IPAddr)))
					return false;
				successFlag |= 0x20;
				break;
			case MSGELEMTYPE_CONTROLLER_MAC_ADDR:
				if(successFlag & 0x40) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseControllerMACAddr(&completeMsg, len, recvControllerInfo.MACAddr)))
					return false;
				successFlag |= 0x40;
				break;
			case MSGELEMTYPE_CONTROLLER_NEXTSEQ:
				if(successFlag & 0x80) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseControllerNextSeq(&completeMsg, len, &recvControllerSeqNum)))
					return false;
				successFlag |= 0x80;
				break;
			
			default:
				APParseUnrecognizedMsgElem(&completeMsg, len);
				APErrorLog("Unrecognized Message Element");
				// return APErrorRaise(AP_ERROR_INVALID_FORMAT,
				// 	"APParseRegisterResponse()");
		}
	}

	if(successFlag != 0xFF && failureFlag != 0x03) { //incomplete message
		APErrorLog("Incomplete Message Element in Register Response");
		return false;
	}

	/* reject */
	if(result == RESULT_FAILURE)
	{
		/* If rejected, The message should contain only result & reason element */
		if(failureFlag != 0x03 || successFlag != 0x01) {
			APErrorLog("The Message carrying some wrong information");
			return false;
		}
		rejected = true;
		switch(reason) {
			case REASON_INVALID_VERSION:
				APLog("Controller rejected the Register Request, the reason is protocol version does not match");
				return false;
			// case REASON_REPEATED_REGISTER:
			// 	APLog("Controller rejected the Register Request, the reason is duplicated service request");
			// 	return false;
			case REASON_INSUFFICIENT_RESOURCE:
				APLog("Controller rejected the Register Request, because there is no enough resources");
				return false;
			default:
				APLog("Controller rejected the Register Request, for unknown reasons");
				return false;
		}
		return false;
	}

	// if(result == RESULT_UNRECOGNIZED_ELEM) { //TODO: RESULT_UNRECOGNIZED_ELEM
	// 	APErrorLog("This Result is not currently being processed");
	//     return false;
	// }

	/* accept */
	/* If accepted, The message should contain only some specified element */
	if(successFlag != 0xFF || failureFlag != 0x01) {
		APErrorLog("The Message carrying some wrong information");
		return false;
	}

	if(ap_register_service != recvRegisteredService) {
		APErrorLog("The service is different from the requested one");
		return false;
	}

	if(controlVal.apid != recvAPID) {
		APErrorLog("The apid in message is different from the one in message header");
	}

	if(controller_name == NULL || strcmp(controller_name, recvControllerInfo.name) != 0) {
		if(ap_discovery_type == DISCOVERY_TPYE_DISCOVERY) APErrorLog("The Controller name has changed");
		free_object(controller_name);
		create_object(controller_name, (strlen(recvControllerInfo.name) + 1), return false;);
		copy_memory(controller_name, recvControllerInfo.name, strlen(recvControllerInfo.name));
		controller_name[strlen(recvControllerInfo.name)] = '\0';
	}

	if(controller_des == NULL || strcmp(controller_des, recvControllerInfo.descriptor) != 0) {
		if(ap_discovery_type == DISCOVERY_TPYE_DISCOVERY) APErrorLog("The Controller descriptor has changed");
		free_object(controller_des);
		create_object(controller_des, (strlen(recvControllerInfo.descriptor) + 1), return false;);
		copy_memory(controller_des, recvControllerInfo.descriptor, strlen(recvControllerInfo.descriptor));
		controller_des[strlen(recvControllerInfo.descriptor)] = '\0';
	}

	if(controller_ip != recvControllerInfo.IPAddr) {
		APErrorLog("The Controller IP Addr has changed");
		return false;
	}

	if(
		controller_mac[0] != recvControllerInfo.MACAddr[0] ||
		controller_mac[1] != recvControllerInfo.MACAddr[1] ||
		controller_mac[2] != recvControllerInfo.MACAddr[2] ||
		controller_mac[3] != recvControllerInfo.MACAddr[3] ||
		controller_mac[4] != recvControllerInfo.MACAddr[4] ||
		controller_mac[5] != recvControllerInfo.MACAddr[5]
	) 
	{
		int i;
		if(ap_discovery_type == DISCOVERY_TPYE_DISCOVERY) APErrorLog("The Controller MAC Addr has changed");
		for(i = 0; i < 6; i++) {
			controller_mac[i] = recvControllerInfo.MACAddr[i];
		}
	}

	return true;
}

bool APReceiveRegisterResponse() 
{
	char buf[BUFFER_SIZE];
	APNetworkAddress addr;
	int readBytes;
	u32 recvAddr;

	recvAPID = recvRegisteredService = 0;
	
	/* receive the datagram */
	if(!(APNetworkReceive(buf,
					 BUFFER_SIZE - 1,
					 &addr,
					 &readBytes))) {
		APErrorLog("Receive Register Response failed");
		return false;
	}

	recvAddr = ntohl(addr.sin_addr.s_addr);
	/* verify the source of the message */
	if(recvAddr != controller_ip) {
		APErrorLog("Message from the illegal source address");
		return false;
	}
	
	/* check if it is a valid Register Response */
	if(!(APParseRegisterResponse(buf, readBytes, ap_seqnum))) {
		return false;
	}

	if(rejected) return false;
	APDebugLog(3, "Accept Register Response");
	
	return true;
}

bool APReadRegisterResponse() 
{

	struct timeval timeout, new_timeout, before, after, delta;
	
	timeout.tv_sec = new_timeout.tv_sec = gRegisterInterval;
	timeout.tv_usec = new_timeout.tv_usec = 0;

	gettimeofday(&before, NULL); // set current time
	

	while(1) 
	{
		bool success = false;

		if(APNetworkTimedPollRead(gSocket, &new_timeout)) 
		{
			success = true;
		}

		if(!success) goto ap_time_over;

		
		if((APReceiveRegisterResponse())) {
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
		APDebugLog(3, "Timer expired during read register response");
	
	APLog("There is no valid Response");
	return false;
}

APStateTransition APEnterRegister() 
{
	APLog("");	
	APLog("######### Register State #########");

	gRegisterCount = 0;
	gMaxRegister = max_retransmit;
	gRegisterInterval = retransmit_interval;

	if(!APNetworkInitControllerAddr(controller_ip)) {
		APErrorLog("Init singlecast socket failed");
		return AP_ENTER_DOWN;
	}

	while(1)
	{
		if(gRegisterCount == gMaxRegister) {
			APLog("No Register Responses for 3 times");
			ap_seqnum_inc();
			return AP_ENTER_DOWN;
		}
		
		APProtocolMessage sendMsg;
		AP_INIT_PROTOCOL(sendMsg);
		APDebugLog(3, "Assemble Register Request");
		if(!(APAssembleRegisterRequest(&sendMsg))) {
			APErrorLog("Failed to assemble Register Request");
			return AP_ENTER_DOWN;
		}
		APLog("Send Register Request");
		if(!(APNetworkSend(sendMsg))) {
			APErrorLog("Failed to send Register Request");
			return AP_ENTER_DOWN;
		}
		AP_FREE_PROTOCOL_MESSAGE(sendMsg);

		gRegisterCount++;
		APDebugLog(3, "The number of REGISTER operations = %d", gRegisterCount);

		/* wait for Responses */
		if(!(APReadRegisterResponse())) {
			if(rejected) return AP_ENTER_DOWN; //rejected, do not need to repeated request
			gRegisterInterval *= 2;
			if(gRegisterInterval > (keepalive_interval / 2)) {
				gRegisterInterval = keepalive_interval / 2;
			}
			APDebugLog(5, "Adjust the register interval to %d sec", gRegisterInterval);
			continue; // no response or invalid response
		}

		//set apid
		APLog("Accept valid Register Response and assigned APID is %d", recvAPID);
		APLog("Controller Next Seq Num is %d", recvControllerSeqNum);
		ap_apid = recvAPID;
		controller_seqnum = recvControllerSeqNum;
		APLog("Registered service successfully");
		break;
	}

	ap_seqnum_inc();
	APLog("The register state is finished");
	return AP_ENTER_RUN;
}
