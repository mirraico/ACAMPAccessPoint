#include "common.h"
#include "error.h"
#include "log.h"
#include "ap.h"
#include "network.h"

int gConfigureCount;
int gMaxConfigure = 3;
int gConfigureInterval = 2;

APBool APAssembleConfigurationReport(APProtocolMessage *messagesPtr)
{
    int k = -1;
	if(messagesPtr == NULL) APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleRegisterRequest()");
	
    APProtocolMessage *msgElems;
	int msgElemCount = 5;
    if(gSecurityOption == SECURITY_WEP || gSecurityOption == SECURITY_WPA) { //WEP or WPA
        msgElemCount++;
    }
    AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleRegisterRequest()"););

    if(
	   (!(APAssembleSSID(&(msgElems[++k])))) ||
	   (!(APAssembleChannel(&(msgElems[++k])))) ||
	   (!(APAssembleHardwareMode(&(msgElems[++k])))) ||
	   (!(APAssembleSuppressSSID(&(msgElems[++k])))) ||
	   (!(APAssembleSecurityOption(&(msgElems[++k]))))
	   )
	{
		int i;
		for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		AP_FREE_OBJECT(msgElems);
		return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
	}

    if(gSecurityOption == SECURITY_WEP) 
    {
        if(!(APAssembleWEP(&(msgElems[++k])))) {
            AP_FREE_PROTOCOL_MESSAGE(msgElems[k]);
            AP_FREE_OBJECT(msgElems);
		    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
        }
    } 
    else if(gSecurityOption == SECURITY_WPA) 
    {
        if(!(APAssembleWPA(&(msgElems[++k])))) {
            AP_FREE_PROTOCOL_MESSAGE(msgElems[k]);
            AP_FREE_OBJECT(msgElems);
		    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
        }
    }

    return APAssembleControlMessage(messagesPtr, 
				 APGetAPID(),
				 APGetSeqNum(),
				 MSGTYPE_CONFIGURATION_REPORT,
				 msgElems,
				 msgElemCount
	);
}

APStateTransition APEnterConfigure() 
{
    APLog("");	
	APLog("######### Configure State #########");

    gConfigureCount = 0;

    AP_REPEAT_FOREVER
    {
        if(gConfigureCount == gMaxConfigure) {
            APLog("No Configuration Update Request for 3 times");
			return AP_ENTER_DOWN;
        }

        APProtocolMessage sendMsg;
        AP_INIT_PROTOCOL(sendMsg);
	    APDebugLog(3, "Assemble Configuration Report");
        if(!APErr(APAssembleConfigurationReport(&sendMsg))) {
            APErrorLog("Failed to assemble Configuration Report");
		    return AP_ENTER_DOWN;
        }
	    APLog("Send Configuration Report");
        if(!APErr(APNetworkSend(sendMsg))) {
            APErrorLog("Failed to send Configuration Report");
		    return AP_ENTER_DOWN;
        }
        AP_FREE_PROTOCOL_MESSAGE(sendMsg);
        // APSeqNumIncrement(); //report do not increase seq num
    }
}