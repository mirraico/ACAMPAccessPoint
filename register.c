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
int gMaxRegister = 3;
int gRegisterInterval = 2;

u16 recvAPID;
u8 recvRegisteredService;


APBool APAssembleRegisterRequest(APProtocolMessage *messagesPtr)
{
    int k = -1;
	if(messagesPtr == NULL) APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleRegisterRequest()");
	
	APProtocolMessage *msgElems;
	int msgElemCount = 6;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleRegisterRequest()"););

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
		return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
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
		return APErrorRaise(AP_ERROR_WRONG_ARG, "APParseRegisterResponse()");
	
	APLog("Parse Register Response");
	
	completeMsg.msg = msg;
	completeMsg.offset = 0;
	
	if(!(APParseControlHeader(&completeMsg, &controlVal))) {
        APErrorLog("Failed to parse header");
        return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
    }
	
	/* not as expected */
    if(controlVal.version != CURRENT_VERSION || controlVal.type != TYPE_CONTROL) {
        APErrorLog("ACAMP version or type is not Expected");
        return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseRegisterResponse()");
    }
    if(controlVal.seqNum != currentSeqNum) {
        APErrorLog("Sequence Number of Response doesn't match Request");
        return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseRegisterResponse()");
    }
	if(controlVal.msgType != MSGTYPE_REGISTER_RESPONSE) {
        APErrorLog("Message is not Register Response as Expected");
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseRegisterResponse()");
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
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
				if(
					result != RESULT_SUCCESS &&
					result != RESULT_FAILURE &&
					result != RESULT_UNRECOGNIZED_ELEM
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
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
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
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
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
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                successFlag |= 0x04;
				break;
			case MSGELEMTYPE_CONTROLLER_NAME:
                if(successFlag & 0x08) {
                    APParseRepeatedMsgElem(&completeMsg, len);
                    APErrorLog("Repeated Message Element");
                    break;
                }
				if(!(APParseControllerName(&completeMsg, len, &recvControllerInfo.name)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                successFlag |= 0x08;
				break;
            case MSGELEMTYPE_CONTROLLER_DESCRIPTOR:
                if(successFlag & 0x10) {
                    APParseRepeatedMsgElem(&completeMsg, len);
                    APErrorLog("Repeated Message Element");
                    break;
                }
				if(!(APParseControllerDescriptor(&completeMsg, len, &recvControllerInfo.descriptor)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                successFlag |= 0x10;
				break;
            case MSGELEMTYPE_CONTROLLER_IP_ADDR:
                if(successFlag & 0x20) {
                    APParseRepeatedMsgElem(&completeMsg, len);
                    APErrorLog("Repeated Message Element");
                    break;
                }
				if(!(APParseControllerIPAddr(&completeMsg, len, &recvControllerInfo.IPAddr)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                successFlag |= 0x20;
				break;
            case MSGELEMTYPE_CONTROLLER_MAC_ADDR:
                if(successFlag & 0x40) {
                    APParseRepeatedMsgElem(&completeMsg, len);
                    APErrorLog("Repeated Message Element");
                    break;
                }
				if(!(APParseControllerMACAddr(&completeMsg, len, recvControllerInfo.MACAddr)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                successFlag |= 0x40;
				break;
			
			default:
                APParseUnrecognizedMsgElem(&completeMsg, len);
                APErrorLog("Unrecognized Message Element");
				// return APErrorRaise(AP_ERROR_INVALID_FORMAT,
				// 	"APParseRegisterResponse()");
		}
	}

	if(successFlag != 0x7F && failureFlag != 0x03) { //incomplete message
        APErrorLog("Incomplete Message Element in Register Response");
        return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseRegisterResponse()");
    }

	/* reject */
	if(result == RESULT_FAILURE)
	{
		/* If rejected, The message should contain only result & reason element */
		if(failureFlag != 0x03 || successFlag != 0x01) {
			APErrorLog("The Message carrying some wrong information");
			return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseRegisterResponse()");
		}
		rejected = AP_TRUE;
		switch(reason) {
			case REASON_INVALID_VERSION:
				APLog("Controller rejected the Register Request, the reason is protocol version does not match");
				return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
			case REASON_REPEATED_REGISTER:
				APLog("Controller rejected the Register Request, the reason is duplicated service request");
				return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
			case REASON_INSUFFICIENT_RESOURCE:
				APLog("Controller rejected the Register Request, because there is no enough resources");
				return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
			default:
				APLog("Controller rejected the Register Request, for unknown reasons");
				return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
		}
		return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
	}

	if(result == RESULT_UNRECOGNIZED_ELEM) { //TODO: RESULT_UNRECOGNIZED_ELEM
		APErrorLog("This Result is not currently being processed");
        return APErrorRaise(AP_ERROR_GENERAL, "APParseRegisterResponse()");
	}

	/* accept */
	/* If accepted, The message should contain only some specified element */
	if(successFlag != 0x7F || failureFlag != 0x01) {
		APErrorLog("The Message carrying some wrong information");
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseRegisterResponse()");
	}

	if(gRegisteredService != recvRegisteredService) {
        APErrorLog("The service is different from the requested one");
		return APErrorRaise(AP_ERROR_WARNING, "APParseRegisterResponse()");
	}

	if(gControllerName == NULL || strcmp(gControllerName, recvControllerInfo.name) != 0) {
		if(gDiscoveryType == DISCOVERY_TPYE_DISCOVERY) APErrorLog("The Controller name has changed");
		AP_FREE_OBJECT(gControllerName);
		AP_CREATE_OBJECT_SIZE_ERR(gControllerName, (strlen(recvControllerInfo.name) + 1), return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APParseRegisterResponse()"););
		AP_COPY_MEMORY(gControllerName, recvControllerInfo.name, strlen(recvControllerInfo.name));
		gControllerName[strlen(recvControllerInfo.name)] = '\0';
	}

	if(gControllerDescriptor == NULL || strcmp(gControllerDescriptor, recvControllerInfo.descriptor) != 0) {
		if(gDiscoveryType == DISCOVERY_TPYE_DISCOVERY) APErrorLog("The Controller descriptor has changed");
		AP_FREE_OBJECT(gControllerDescriptor);
		AP_CREATE_OBJECT_SIZE_ERR(gControllerDescriptor, (strlen(recvControllerInfo.descriptor) + 1), return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APParseRegisterResponse()"););
		AP_COPY_MEMORY(gControllerDescriptor, recvControllerInfo.descriptor, strlen(recvControllerInfo.descriptor));
		gControllerDescriptor[strlen(recvControllerInfo.descriptor)] = '\0';
	}

	if(gControllerIPAddr != recvControllerInfo.IPAddr) {
		APErrorLog("The Controller IP Addr has changed");
		return APErrorRaise(AP_ERROR_WARNING, "APParseRegisterResponse()");
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
	if(!APErr(APNetworkReceive(buf,
					 AP_BUFFER_SIZE - 1,
                     &addr,
					 &readBytes))) {
        APErrorLog("Receive Register Response failed");
		return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
	}

	recvAddr = ntohl(addr.sin_addr.s_addr);
    /* verify the source of the message */
    if(recvAddr != gControllerIPAddr) {
        APErrorLog("Message from the illegal source address");
        return APErrorRaise(AP_ERROR_WARNING, "APReceiveRegisterResponse()");
    }
	
	/* check if it is a valid Register Response */
	if(!APErr(APParseRegisterResponse(buf, readBytes, APGetSeqNum()))) {
		return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
	}

	if(rejected) return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
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
        if(APNetworkTimedPollRead(gSocket, &new_timeout)) 
        {
            /* if there was no error, raise a "success error", 
            so we can easily handle all the cases in the switch */
            APErrorRaise(AP_ERROR_SUCCESS, NULL);
        }

        switch(APGetLastErrorCode())
        {
            case AP_ERROR_TIME_EXPIRED:
                goto ap_time_over;
                break;
                
            case AP_ERROR_SUCCESS:
                if(APErr(APReceiveRegisterResponse())) {
                    return AP_TRUE;
                }
				/* if rejected, break */
				if(rejected) {
					return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
				}
				/* if failed, do not break, compute time and go on */
            case AP_ERROR_INTERRUPTED: 
                gettimeofday(&after, NULL);
                APTimevalSubtract(&delta, &after, &before);
                if(APTimevalSubtract(&new_timeout, &timeout, &delta) == 1) { 
					/* time is over (including receive & pause) */
					goto ap_time_over;
				}
                break;
            default:
                return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
        }
    }
    ap_time_over:
        APDebugLog(3, "Timer expired during read register response");
	
	APLog("There is no valid Response");
    return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
}

APStateTransition APEnterRegister() 
{
    APLog("");	
	APLog("######### Register State #########");

    gRegisterCount = 0;

    if(!APNetworkInitControllerAddr(gControllerIPAddr)) {
        APErrorLog("Init singlecast socket failed");
		return AP_ENTER_DOWN;
	}

    AP_REPEAT_FOREVER
    {
        if(gRegisterCount == gMaxRegister) {
            APLog("No Register Responses for 3 times");
			return AP_ENTER_DOWN;
        }
        
        APProtocolMessage sendMsg;
        AP_INIT_PROTOCOL(sendMsg);
	    APDebugLog(3, "Assemble Register Request");
        if(!APErr(APAssembleRegisterRequest(&sendMsg))) {
            APErrorLog("Failed to assemble Register Request");
		    return AP_ENTER_DOWN;
        }
	    APLog("Send Register Request");
        if(!APErr(APNetworkSend(sendMsg))) {
            APErrorLog("Failed to send Register Request");
		    return AP_ENTER_DOWN;
        }
        AP_FREE_PROTOCOL_MESSAGE(sendMsg);

        gRegisterCount++;
	    APDebugLog(3, "The number of REGISTER operations = %d", gRegisterCount);

        /* wait for Responses */
        if(!APErr(APReadRegisterResponse())) {
        	APSeqNumIncrement();
			if(rejected) return AP_ENTER_DOWN; //rejected, do not need to repeated request
			continue; // no response or invalid response
		}
        APSeqNumIncrement();

        //set apid
    	APLog("Accept valid Register Response and assigned APID is %d", recvAPID);
        APSetAPID(recvAPID);
        APLog("Registered service successfully");
        break;
    }

    APLog("The register state is finished");
    return AP_ENTER_RUN;
}
