#include "common.h"
#include "error.h"
#include "log.h"
#include "ap.h"
#include "network.h"


int gRegisterCount;
int gMaxRegister = 3;
int gRegisterInterval = 2;

u16 tempAPID;


APBool APAssembleRegisterRequest(APProtocolMessage *messagesPtr)
{
    int k = -1;
	if(messagesPtr == NULL) APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	APProtocolMessage *msgElems;
	int msgElemCount = 6;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););

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
	
	if(msg == NULL) 
		return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	APDebugLog(3, "Parse Register Response");
	
	completeMsg.msg = msg;
	completeMsg.offset = 0;
	
	if(!(APParseControlHeader(&completeMsg, &controlVal))) {
        return APErrorRaise(AP_ERROR_INVALID_FORMAT, "Unrecognized Message");
    }
	
	/* not as expected */
    if(controlVal.version != CURRENT_VERSION || controlVal.type != TYPE_CONTROL)
        return APErrorRaise(AP_ERROR_INVALID_FORMAT, "ACAMP version or type is wrong");
    if(controlVal.seqNum != currentSeqNum) 
        return APErrorRaise(AP_ERROR_INVALID_FORMAT, "Sequence Number of Response doesn't match Request");
	if(controlVal.msgType != MSGTYPE_REGISTER_RESPONSE)
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, "Message is not Register Response as Expected");

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
				if(!(APParseResultCode(&completeMsg, len, &result)))
                    return APErrorRaise(AP_ERROR_INVALID_FORMAT, NULL);
				break;
			case MSGELEMTYPE_REASON_CODE:
				if(!(APParseReasonCode(&completeMsg, len, &reason)))
                    return APErrorRaise(AP_ERROR_INVALID_FORMAT, NULL);
				break;
			case MSGELEMTYPE_ASSIGNED_APID:
				if(!(APParseAssignedAPID(&completeMsg, len, &tempAPID)))
                    return APErrorRaise(AP_ERROR_INVALID_FORMAT, NULL);
				break;
			case MSGELEMTYPE_CONTROLLER_NAME:
				if(!(APParseControllerName(&completeMsg, len, &gControllerName)))
                    return APErrorRaise(AP_ERROR_INVALID_FORMAT, NULL);
				break;
            case MSGELEMTYPE_CONTROLLER_DESCRIPTOR:
				if(!(APParseControllerDescriptor(&completeMsg, len, &gControllerDescriptor)))
                    return APErrorRaise(AP_ERROR_INVALID_FORMAT, NULL);
				break;
            case MSGELEMTYPE_CONTROLLER_IP_ADDR:
				if(!(APParseControllerIPAddr(&completeMsg, len, &gControllerIPAddr)))
                    return APErrorRaise(AP_ERROR_INVALID_FORMAT, NULL);
				break;
            case MSGELEMTYPE_CONTROLLER_MAC_ADDR:
				if(!(APParseControllerMACAddr(&completeMsg, len, gControllerMACAddr)))
                    return APErrorRaise(AP_ERROR_INVALID_FORMAT, NULL);
				break;
			
			default:
				return APErrorRaise(AP_ERROR_INVALID_FORMAT,
					"Unrecognized Message Element");
		}
	}
	if(result != 0) {
		APLog("Refuse Register");
		return APErrorRaise(AP_ERROR_NONE, NULL);
	}
	return AP_TRUE;
}

APBool APReceiveRegisterResponse() 
{
	char buf[AP_BUFFER_SIZE];
	APNetworkAddress addr;
	int readBytes;
	
	/* receive the datagram */
	if(!APErr(APNetworkReceive(buf,
					 AP_BUFFER_SIZE - 1,
                     &addr,
					 &readBytes))) {
        APErrorLog("Receive register response failed");
		return AP_FALSE;
	}

    /* verify the source of the message */
    if(addr.sin_addr.s_addr != gControllerIPAddr) {
        APErrorLog("Illegal sources");
        return APErrorRaise(AP_ERROR_GENERAL, NULL);
    }
	
	/* check if it is a valid Register Response */
	if(!APErr(APParseRegisterResponse(buf, readBytes, APGetWaitSeqNum()))) {
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, 
				    "Received something different from a\
				     Register Response while in Register State");
	}

	/* verify gControllerIPAddr (maybe change) */
    if(addr.sin_addr.s_addr != gControllerIPAddr) {
        APErrorLog("Illegal sources");
        return APErrorRaise(AP_ERROR_GENERAL, NULL);
    }

    APLog("Receive Register Response");
	
	return AP_TRUE;
}

APBool APReadRegisterResponse() {

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
                if(APReceiveRegisterResponse()) {
                    goto ap_recieve_success;
                }
				/* if failed, no break, compute time and go on */
            case AP_ERROR_INTERRUPTED: 
                gettimeofday(&after, NULL);
                APTimevalSubtract(&delta, &after, &before);
                if(APTimevalSubtract(&new_timeout, &timeout, &delta) == 1) { 
					/* time is over (including receive & pause) */
					goto ap_time_over;
				}
        }
    }
    ap_time_over:
        APDebugLog(3, "Timer expired during read register response");
    ap_recieve_success:
    
    return AP_TRUE;
}

APStateTransition APEnterRegister() 
{
    APLog("");	
	APLog("######### Register State #########");

    gRegisterCount = 0;

    if(!APNetworkInitControllerAddr(gControllerIPAddr)) {
        APErrorLog("Init singlecast failed");
		return AP_ENTER_DOWN;
	}

    AP_REPEAT_FOREVER
    {
        if(gRegisterCount == gMaxRegister) {
            APLog("No register responses for 3 times");
			return AP_ENTER_DOWN;
        }
        
        APProtocolMessage sendMsg;
        AP_INIT_PROTOCOL(sendMsg);
	    APDebugLog(3, "Assemble register request");
        if(!APErr(APAssembleRegisterRequest(&sendMsg))) {
            APErrorLog("Assemble register request failed");
		    return AP_ENTER_DOWN;
        }
	    APLog("Send register request");
        if(!APNetworkSend(sendMsg)) {
            APErrorLog("Send register request failed");
		    return AP_ENTER_DOWN;
        }
        AP_FREE_PROTOCOL_MESSAGE(sendMsg);
        APSeqNumIncrement();

        gRegisterCount++;
	    APDebugLog(3, "Register count = %d", gRegisterCount);

        /* wait for Responses */
        if(!APReadRegisterResponse()) {
			continue; // no response
		}
		APWaitSeqNumIncrement();

        //set apid
        APSetAPID(tempAPID);
        APLog("Registered Successfully");
        break;
    }

    return AP_ENTER_CONFIGURE;
}
