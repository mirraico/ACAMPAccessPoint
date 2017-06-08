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

APBool APAssembleRegisterRequest(APProtocolMessage *messagesPtr)
{
	int k = -1;
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 6;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return AP_FALSE;);

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
		AP_FREE_OBJECT(msgElems);
		return AP_FALSE;
	}
	
	return APAssembleControlMessage(messagesPtr, 
				 APGetAPID(),
				 APGetSeqNum(),
				 MSGTYPE_REGISTER_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

APBool APParseRegisterResponse(char *msg, 
					   int len,
					   int currentSeqNum) 
{
	
	u16 result, reason;
	APHeaderVal controlVal;
	APProtocolMessage completeMsg;
	controllerVal recvControllerInfo;

	u16 successFlag = 0;
	u16 failureFlag = 0;

	rejected = AP_FALSE;
	
	if(msg == NULL) 
		return AP_FALSE;
	
	APLog("Parse Register Response");
	
	completeMsg.msg = msg;
	completeMsg.offset = 0;
	
	if(!(APParseControlHeader(&completeMsg, &controlVal))) {
		APErrorLog("Failed to parse header");
		return AP_FALSE;
	}
	
	/* not as expected */
	if(controlVal.version != CURRENT_VERSION || controlVal.type != TYPE_CONTROL) {
		APErrorLog("ACAMP version or type is not Expected");
		return AP_FALSE;
	}
	if(controlVal.seqNum != currentSeqNum) {
		APErrorLog("Sequence Number of Response doesn't match Request");
		return AP_FALSE;
	}
	if(controlVal.msgType != MSGTYPE_REGISTER_RESPONSE) {
		APErrorLog("Message is not Register Response as Expected");
		return AP_FALSE;
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
					return AP_FALSE;
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
					return AP_FALSE;
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
					return AP_FALSE;
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
					return AP_FALSE;
				successFlag |= 0x04;
				break;
			case MSGELEMTYPE_CONTROLLER_NAME:
				if(successFlag & 0x08) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseControllerName(&completeMsg, len, &recvControllerInfo.name)))
					return AP_FALSE;
				successFlag |= 0x08;
				break;
			case MSGELEMTYPE_CONTROLLER_DESCRIPTOR:
				if(successFlag & 0x10) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseControllerDescriptor(&completeMsg, len, &recvControllerInfo.descriptor)))
					return AP_FALSE;
				successFlag |= 0x10;
				break;
			case MSGELEMTYPE_CONTROLLER_IP_ADDR:
				if(successFlag & 0x20) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseControllerIPAddr(&completeMsg, len, &recvControllerInfo.IPAddr)))
					return AP_FALSE;
				successFlag |= 0x20;
				break;
			case MSGELEMTYPE_CONTROLLER_MAC_ADDR:
				if(successFlag & 0x40) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseControllerMACAddr(&completeMsg, len, recvControllerInfo.MACAddr)))
					return AP_FALSE;
				successFlag |= 0x40;
				break;
			case MSGELEMTYPE_CONTROLLER_NEXTSEQ:
				if(successFlag & 0x80) {
					APParseRepeatedMsgElem(&completeMsg, len);
					APErrorLog("Repeated Message Element");
					break;
				}
				if(!(APParseControllerNextSeq(&completeMsg, len, &recvControllerSeqNum)))
					return AP_FALSE;
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
		return AP_FALSE;
	}

	/* reject */
	if(result == RESULT_FAILURE)
	{
		/* If rejected, The message should contain only result & reason element */
		if(failureFlag != 0x03 || successFlag != 0x01) {
			APErrorLog("The Message carrying some wrong information");
			return AP_FALSE;
		}
		rejected = AP_TRUE;
		switch(reason) {
			case REASON_INVALID_VERSION:
				APLog("Controller rejected the Register Request, the reason is protocol version does not match");
				return AP_FALSE;
			// case REASON_REPEATED_REGISTER:
			// 	APLog("Controller rejected the Register Request, the reason is duplicated service request");
			// 	return AP_FALSE;
			case REASON_INSUFFICIENT_RESOURCE:
				APLog("Controller rejected the Register Request, because there is no enough resources");
				return AP_FALSE;
			default:
				APLog("Controller rejected the Register Request, for unknown reasons");
				return AP_FALSE;
		}
		return AP_FALSE;
	}

	// if(result == RESULT_UNRECOGNIZED_ELEM) { //TODO: RESULT_UNRECOGNIZED_ELEM
	// 	APErrorLog("This Result is not currently being processed");
	//     return AP_FALSE;
	// }

	/* accept */
	/* If accepted, The message should contain only some specified element */
	if(successFlag != 0xFF || failureFlag != 0x01) {
		APErrorLog("The Message carrying some wrong information");
		return AP_FALSE;
	}

	if(gRegisteredService != recvRegisteredService) {
		APErrorLog("The service is different from the requested one");
		return AP_FALSE;
	}

	if(controlVal.apid != recvAPID) {
		APErrorLog("The apid in message is different from the one in message header");
	}

	if(gControllerName == NULL || strcmp(gControllerName, recvControllerInfo.name) != 0) {
		if(gDiscoveryType == DISCOVERY_TPYE_DISCOVERY) APErrorLog("The Controller name has changed");
		AP_FREE_OBJECT(gControllerName);
		AP_CREATE_OBJECT_SIZE_ERR(gControllerName, (strlen(recvControllerInfo.name) + 1), return AP_FALSE;);
		AP_COPY_MEMORY(gControllerName, recvControllerInfo.name, strlen(recvControllerInfo.name));
		gControllerName[strlen(recvControllerInfo.name)] = '\0';
	}

	if(gControllerDescriptor == NULL || strcmp(gControllerDescriptor, recvControllerInfo.descriptor) != 0) {
		if(gDiscoveryType == DISCOVERY_TPYE_DISCOVERY) APErrorLog("The Controller descriptor has changed");
		AP_FREE_OBJECT(gControllerDescriptor);
		AP_CREATE_OBJECT_SIZE_ERR(gControllerDescriptor, (strlen(recvControllerInfo.descriptor) + 1), return AP_FALSE;);
		AP_COPY_MEMORY(gControllerDescriptor, recvControllerInfo.descriptor, strlen(recvControllerInfo.descriptor));
		gControllerDescriptor[strlen(recvControllerInfo.descriptor)] = '\0';
	}

	if(gControllerIPAddr != recvControllerInfo.IPAddr) {
		APErrorLog("The Controller IP Addr has changed");
		return AP_FALSE;
	}

	if(
		gControllerMACAddr[0] != recvControllerInfo.MACAddr[0] ||
		gControllerMACAddr[1] != recvControllerInfo.MACAddr[1] ||
		gControllerMACAddr[2] != recvControllerInfo.MACAddr[2] ||
		gControllerMACAddr[3] != recvControllerInfo.MACAddr[3] ||
		gControllerMACAddr[4] != recvControllerInfo.MACAddr[4] ||
		gControllerMACAddr[5] != recvControllerInfo.MACAddr[5]
	) 
	{
		int i;
		if(gDiscoveryType == DISCOVERY_TPYE_DISCOVERY) APErrorLog("The Controller MAC Addr has changed");
		for(i = 0; i < 6; i++) {
			gControllerMACAddr[i] = recvControllerInfo.MACAddr[i];
		}
	}

	return AP_TRUE;
}

APBool APReceiveRegisterResponse() 
{
	char buf[AP_BUFFER_SIZE];
	APNetworkAddress addr;
	int readBytes;
	u32 recvAddr;

	recvAPID = recvRegisteredService = 0;
	
	/* receive the datagram */
	if(!(APNetworkReceive(buf,
					 AP_BUFFER_SIZE - 1,
					 &addr,
					 &readBytes))) {
		APErrorLog("Receive Register Response failed");
		return AP_FALSE;
	}

	recvAddr = ntohl(addr.sin_addr.s_addr);
	/* verify the source of the message */
	if(recvAddr != gControllerIPAddr) {
		APErrorLog("Message from the illegal source address");
		return AP_FALSE;
	}
	
	/* check if it is a valid Register Response */
	if(!(APParseRegisterResponse(buf, readBytes, APGetSeqNum()))) {
		return AP_FALSE;
	}

	if(rejected) return AP_FALSE;
	APDebugLog(3, "Accept Register Response");
	
	return AP_TRUE;
}

APBool APReadRegisterResponse() 
{

	struct timeval timeout, new_timeout, before, after, delta;
	
	timeout.tv_sec = new_timeout.tv_sec = gRegisterInterval;
	timeout.tv_usec = new_timeout.tv_usec = 0;

	gettimeofday(&before, NULL); // set current time
	

	AP_REPEAT_FOREVER 
	{
		APBool success = false;

		if(APNetworkTimedPollRead(gSocket, &new_timeout)) 
		{
			success = true;
		}

		if(!success) goto ap_time_over;

		
		if((APReceiveRegisterResponse())) {
			return AP_TRUE;
		}

		/* if rejected, break */
		if(rejected) {
			return AP_FALSE;
		}

		/* compute time and go on */
		gettimeofday(&after, NULL);
		APTimevalSubtract(&delta, &after, &before);
		if(APTimevalSubtract(&new_timeout, &timeout, &delta) == 1) { 
			/* time is over (including receive & pause) */
			goto ap_time_over;
		}
	}
	ap_time_over:
		APDebugLog(3, "Timer expired during read register response");
	
	APLog("There is no valid Response");
	return AP_FALSE;
}

APStateTransition APEnterRegister() 
{
	APLog("");	
	APLog("######### Register State #########");

	gRegisterCount = 0;
	gMaxRegister = gMaxRetransmit;
	gRegisterInterval = gRetransmitInterval;

	if(!APNetworkInitControllerAddr(gControllerIPAddr)) {
		APErrorLog("Init singlecast socket failed");
		return AP_ENTER_DOWN;
	}

	AP_REPEAT_FOREVER
	{
		if(gRegisterCount == gMaxRegister) {
			APLog("No Register Responses for 3 times");
			APSeqNumIncrement();
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
			if(gRegisterInterval > (gKeepAliveInterval / 2)) {
				gRegisterInterval = gKeepAliveInterval / 2;
			}
			APDebugLog(5, "Adjust the register interval to %d sec", gRegisterInterval);
			continue; // no response or invalid response
		}

		//set apid
		APLog("Accept valid Register Response and assigned APID is %d", recvAPID);
		APLog("Controller Next Seq Num is %d", recvControllerSeqNum);
		APSetAPID(recvAPID);
		APSetControllerSeqNum(recvControllerSeqNum);
		APLog("Registered service successfully");
		break;
	}

	APSeqNumIncrement();
	APLog("The register state is finished");
	return AP_ENTER_RUN;
}
