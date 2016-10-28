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


APBool APEvaluateController()
{
    int i;
    /* now the strategy is that choose the first */
    AP_CREATE_OBJECT_SIZE_ERR(gControllerName, (strlen(controllers[0].name) + 1), return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
	AP_COPY_MEMORY(gControllerName, &controllers[0].name, strlen(controllers[0].name));
	gControllerName[strlen(controllers[0].name)] = '\0';

    AP_CREATE_OBJECT_SIZE_ERR(gControllerDescriptor, (strlen(controllers[0].descriptor) + 1), return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
	AP_COPY_MEMORY(gControllerDescriptor, &controllers[0].descriptor, strlen(controllers[0].descriptor));
	gControllerDescriptor[strlen(controllers[0].descriptor)] = '\0';

    gControllerIPAddr = controllers[0].IPAddr; 
    for(i = 0; i < 6; i++) {
        gControllerMACAddr[i] = controllers[0].MACAddr[i];
    }
    return AP_TRUE;
}

APBool APAssembleDiscoveryRequest(APProtocolMessage *messagesPtr)
{
	if(messagesPtr == NULL) APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, NULL););
	
	return APAssembleControlMessage(messagesPtr, 
				 APGetAPID(),
				 APGetSeqNum(),
				 MSGTYPE_DISCOVERY_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

APBool APParseDiscoveryResponse(char *msg, 
				       int len,
				       int currentSeqNum,
				       controllerVal *controllerPtr) 
{
	APHeaderVal controlVal;
	APProtocolMessage completeMsg;
	
	if(msg == NULL || controllerPtr == NULL) 
		return APErrorRaise(AP_ERROR_WRONG_ARG, NULL);
	
	APDebugLog(3, "Parse Discovery Response");
	
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
	if(controlVal.msgType != MSGTYPE_DISCOVERY_RESPONSE)
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, "Message is not Discovery Response as Expected");

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
				if(!(APParseControllerName(&completeMsg, len, &(controllerPtr->name))))
                    return APErrorRaise(AP_ERROR_INVALID_FORMAT, NULL);
				break;
            case MSGELEMTYPE_CONTROLLER_DESCRIPTOR:
				if(!(APParseControllerDescriptor(&completeMsg, len, &(controllerPtr->descriptor))))
                    return APErrorRaise(AP_ERROR_INVALID_FORMAT, NULL);
				break;
            case MSGELEMTYPE_CONTROLLER_IP_ADDR:
				if(!(APParseControllerIPAddr(&completeMsg, len, &(controllerPtr->IPAddr))))
                    return APErrorRaise(AP_ERROR_INVALID_FORMAT, NULL);
				break;
            case MSGELEMTYPE_CONTROLLER_MAC_ADDR:
				if(!(APParseControllerMACAddr(&completeMsg, len, &(controllerPtr->MACAddr))))
                    return APErrorRaise(AP_ERROR_INVALID_FORMAT, NULL);
				break;
			
			default:
				return APErrorRaise(AP_ERROR_INVALID_FORMAT,
					"Unrecognized Message Element");
		}
	}
	return AP_TRUE;
}

APBool APReceiveDiscoveryResponse() {
	char buf[AP_BUFFER_SIZE];
	APNetworkAddress addr;
	controllerVal *controllerPtr = &controllers[gDiscoveryCount];
	int readBytes;
	
	/* receive the datagram */
	if(!APErr(APNetworkReceiveFromBroad(buf,
					 AP_BUFFER_SIZE - 1,
                     &addr,
					 &readBytes))) {
        APErrorLog("Receive discovery response failed");
		return AP_FALSE;
	}
	
	/* check if it is a valid Discovery Response */
	if(!APErr(APParseDiscoveryResponse(buf, readBytes, APGetWaitSeqNum(), controllerPtr))) {
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, 
				    "Received something different from a\
				     Discovery Response while in Discovery State");
	}

    /* the address declared by controller is fake */
    if(addr.sin_addr.s_addr != controllers[foundControllerCount].IPAddr) {
        APErrorLog("Address Deception");
        return APErrorRaise(AP_ERROR_GENERAL, NULL);
    }

    APLog("Discovery Response from %u.%u.%u.%u", (u8)(controllers[foundControllerCount].IPAddr >> 24), (u8)(controllers[foundControllerCount].IPAddr >> 16),\
	  (u8)(controllers[foundControllerCount].IPAddr >> 8),  (u8)(controllers[foundControllerCount].IPAddr >> 0));
	
	return AP_TRUE;
}

APBool APReadDiscoveryResponse() 
{

	struct timeval timeout, new_timeout, before, after, delta;
	
	timeout.tv_sec = new_timeout.tv_sec = gDiscoveryInterval;
	timeout.tv_usec = new_timeout.tv_usec = 0;

    gettimeofday(&before, NULL); // set current time
	

    AP_REPEAT_FOREVER 
    {
        if(APNetworkTimedPollRead(gSocketBroad, &new_timeout)) 
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
                if(APReceiveDiscoveryResponse()) {
                    foundControllerCount++;
                }
                /* no break, compute time and go on */
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
        APDebugLog(3, "Timer expired during read discovery response");	

    if(foundControllerCount == 0) return AP_FALSE;
    
    return AP_TRUE;
}

APStateTransition APEnterDiscovery() 
{
    APLog("");	
	APLog("######### Discovery State #########");

    /*reset discovery count*/
    gDiscoveryCount = 0;

    if(!APNetworkInitBroadcast()) {
        APErrorLog("Init broadcast failed");
		return AP_ENTER_DOWN;
	}

    AP_REPEAT_FOREVER
    {
        foundControllerCount = 0;

        if(gDiscoveryCount == gMaxDiscovery) {
            APLog("No discovery responses for 3 times");
			return AP_ENTER_DOWN;
        }
        
        APProtocolMessage sendMsg;
        AP_INIT_PROTOCOL(sendMsg);
	    APDebugLog(3, "Assemble discovery request");
        if(!APErr(APAssembleDiscoveryRequest(&sendMsg))) {
            APErrorLog("Assemble discovery request failed");
		    return AP_ENTER_DOWN;
        }
	    APLog("Send discovery request");
        if(!APNetworkSendToBroad(sendMsg)) {
            APErrorLog("Send discovery request failed");
		    return AP_ENTER_DOWN;
        }
        AP_FREE_PROTOCOL_MESSAGE(sendMsg);
        APSeqNumIncrement();

        gDiscoveryCount++;
	    APDebugLog(3, "Discovery count = %d", gDiscoveryCount);

        /* wait for Responses */
        if(!APReadDiscoveryResponse()) {
			continue; // no response
		}
        APWaitSeqNumIncrement();

        //choose one controller
        APEvaluateController();
        APLog("Picks a Controller from %u.%u.%u.%u", (u8)(gControllerIPAddr >> 24), (u8)(gControllerIPAddr >> 16),\
	        (u8)(gControllerIPAddr >> 8),  (u8)(gControllerIPAddr >> 0));
        break;
    }

    return AP_ENTER_REGISTER;
}
