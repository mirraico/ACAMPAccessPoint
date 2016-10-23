#include "common.h"
#include "error.h"
#include "ap.h"
#include "network.h"

int gDiscoveryCount;
int gMaxDiscovery = 3;
int gDiscoveryInterval = 10;

#define APFoundController()	(gControllerIPAddr != -1)

APBool APAssembleDiscoveryRequest(APProtocolMessage *messagesPtr)
{
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount);
	
	return APAssembleControlMessage(messagesPtr, 
				 APGetAPID(),
				 APGetSeqNum(),
				 MSGTYPE_DISCOVERY_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

APBool APReadDiscoveryResponse() {

	struct timeval timeout;
	
	timeout.tv_sec = gDiscoveryInterval;
	timeout.tv_usec = 0;
	
    if(APNetworkTimedPollRead(gSocketBroad, &timeout)) 
    { 
        APErrorRaise(AP_ERROR_SUCCESS, NULL);
    }

    switch(APGetLastErrorCode())
    {
        case AP_ERROR_TIME_EXPIRED:
            return AP_FALSE;
            
        case AP_ERROR_SUCCESS:
            // return APReceiveDiscoveryResponse();
            break;
    }
}

APStateTransition APEnterDiscovery() 
{
    gDiscoveryCount = 0;

    if(!APNetworkInitBroadcast()){
		return AP_ENTER_DOWN;
	}

    AP_REPEAT_FOREVER
    {
        if(gDiscoveryCount == gMaxDiscovery)
			return AP_ENTER_DOWN;
        
        APProtocolMessage sendMsg;
        AP_INIT_PROTOCOL(sendMsg);
        APAssembleDiscoveryRequest(&sendMsg);
        APNetworkSendToBroad(sendMsg);
        AP_FREE_PROTOCOL_MESSAGE(sendMsg);

        gDiscoveryCount++;
        if(APReadDiscoveryResponse() && APFoundController()) 
        {
			break;
		}
    }

}
