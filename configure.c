#include "common.h"
#include "error.h"
#include "log.h"
#include "ap.h"
#include "network.h"
#include "hostapd.h"

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

APBool APParseConfigurationUpdateRequest(char *msg, int len) 
{
	APHeaderVal controlVal;
	APProtocolMessage completeMsg;

    /* avoiding invalid messages can have an impact on the configuration */
    u16 recvFlag = 0;
    u32 recvControllerSeqNum;
    char* recvSSID = NULL;
    u8 recvChannel;
    u8 recvHardwareMode;
    u8 recvSuppressSSID;
    u8 recvSecurityOption;
    APWEP recvWEP;
    APWPA recvWPA;

    //default vaule
    recvWEP.default_key = 0;
	recvWEP.key0_type = 0;
	recvWEP.key0 = NULL;
	recvWEP.key1_type = 0;
	recvWEP.key1 = NULL;
	recvWEP.key2_type = 0;
	recvWEP.key2 = NULL;
	recvWEP.key3_type = 0;
	recvWEP.key3 = NULL;
	recvWPA.version = 2;
	recvWPA.password = NULL;
	recvWPA.pairwire_cipher = 2;
	recvWPA.group_rekey = 86400;

    if(msg == NULL) 
		return APErrorRaise(AP_ERROR_WRONG_ARG, "APParseConfigurationUpdateRequest()");
    
    APLog("Parse Configuration Update Request");

    completeMsg.msg = msg;
	completeMsg.offset = 0;

    if(!(APParseControlHeader(&completeMsg, &controlVal))) {
        APErrorLog("Failed to parse header");
        return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
    }

    /* not as expected */
    if(controlVal.version != CURRENT_VERSION || controlVal.type != TYPE_CONTROL) {
        APErrorLog("ACAMP version or type is not Expected");
        return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseConfigurationUpdateRequest()");
    }
    if(controlVal.msgType != MSGTYPE_CONFIGURATION_UPDATE_REQUEST) {
        APErrorLog("Message is not Configuration Update Request as Expected");
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseConfigurationUpdateRequest()");
    }

    /* record the seq number of the controller */
    recvControllerSeqNum = controlVal.seqNum; 

    /* parse message elements */
    while(completeMsg.offset < controlVal.msgLen) 
    {
        u16 type = 0;
		u16 len = 0;

        APParseFormatMsgElem(&completeMsg, &type, &len);
	    // APDebugLog(3, "Parsing Message Element: %u, len: %u", type, len);

        switch(type) 
        {
            case MSGELEMTYPE_SSID:
                if(recvFlag & 0x01) {
                    APParseRepeatedMsgElem(&completeMsg, len);
                    APErrorLog("Repeated Message Element");
                    break;
                }
                if(!(APParseSSID(&completeMsg, len, &recvSSID)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                recvFlag |= 0x01;
                break;
            case MSGELEMTYPE_CHANNEL:
                if(recvFlag & 0x02) {
                    APParseRepeatedMsgElem(&completeMsg, len);
                    APErrorLog("Repeated Message Element");
                    break;
                }
                if(!(APParseChannel(&completeMsg, len, &recvChannel)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                recvFlag |= 0x02;
                break;
            case MSGELEMTYPE_HARDWARE_MODE:
                if(recvFlag & 0x04) {
                    APParseRepeatedMsgElem(&completeMsg, len);
                    APErrorLog("Repeated Message Element");
                    break;
                }
                if(!(APParseHardwareMode(&completeMsg, len, &recvHardwareMode)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                recvFlag |= 0x04;
                break;
            case MSGELEMTYPE_SUPPRESS_SSID:
                if(recvFlag & 0x08) {
                    APParseRepeatedMsgElem(&completeMsg, len);
                    APErrorLog("Repeated Message Element");
                    break;
                }
                if(!(APParseSuppressSSID(&completeMsg, len, &recvSuppressSSID)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                recvFlag |= 0x08;
                break;
            case MSGELEMTYPE_SECURITY_OPTION:
                if(recvFlag & 0x10) {
                    APParseRepeatedMsgElem(&completeMsg, len);
                    APErrorLog("Repeated Message Element");
                    break;
                }
                if(!(APParseSecurityOption(&completeMsg, len, &recvSecurityOption)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                recvFlag |= 0x10;
                break;
            case MSGELEMTYPE_WEP_INFO:
                if(recvFlag & 0x20) {
                    APParseRepeatedMsgElem(&completeMsg, len);
                    APErrorLog("Repeated Message Element");
                    break;
                }
                if(!(APParseWEP(&completeMsg, len, &recvWEP)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                recvFlag |= 0x20;
                break;
            case MSGELEMTYPE_WPA_INFO:
                if(recvFlag & 0x40) {
                    APParseRepeatedMsgElem(&completeMsg, len);
                    APErrorLog("Repeated Message Element");
                    break;
                }
                if(!(APParseWPA(&completeMsg, len, &recvWPA)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                recvFlag |= 0x40;
                break;
            default:
                APParseUnrecognizedMsgElem(&completeMsg, len);
                APErrorLog("Unrecognized Message Element");
				// return APErrorRaise(AP_ERROR_INVALID_FORMAT,
				// 	"APParseConfigurationUpdateRequest()");
        }
    }

    /* There are no message errors, so apply the configuration */
    if(recvFlag & 0x01) {
        AP_FREE_OBJECT(gSSID);
		// AP_CREATE_OBJECT_SIZE_ERR(gSSID, (strlen(recvSSID) + 1), return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APParseConfigurationUpdateRequest()"););
		// AP_COPY_MEMORY(gSSID, recvSSID, strlen(recvSSID));
		// gSSID[strlen(recvSSID)] = '\0';
        // AP_FREE_OBJECT(recvSSID);
        gSSID = recvSSID; //There is a new pointer, so it will not cause memory leak
        APDebugLog(3, "Apply new SSID");
    }
    if(recvFlag & 0x02) {
        gChannel = recvChannel;
        APDebugLog(3, "Apply new Channel");
    }
    if(recvFlag & 0x04) {
        gHardwareMode = recvHardwareMode;
        APDebugLog(3, "Apply new Hardware Mode");
    }
    if(recvFlag & 0x08) {
        gSuppressSSID = recvSuppressSSID;
        APDebugLog(3, "Apply new Suppress SSID");
    }
    if(recvFlag & 0x10) {
        gSecurityOption = recvSecurityOption;
        APDebugLog(3, "Apply new Security Option");
    }
    if(recvFlag & 0x20) {
        AP_FREE_OBJECT(gWEP.key0);
        AP_FREE_OBJECT(gWEP.key1);
        AP_FREE_OBJECT(gWEP.key2);
        AP_FREE_OBJECT(gWEP.key3);
        gWEP = recvWEP;
        APDebugLog(3, "Apply new WEP");
    }
    if(recvFlag & 0x40) {
        AP_FREE_OBJECT(gWPA.password);
        gWPA = recvWPA;
        APDebugLog(3, "Apply new WPA");
    }

    gControllerSeqNum = recvControllerSeqNum;
    APDebugLog(5, "Controller Seq Number is %u", gControllerSeqNum);

    return AP_TRUE;
}

APBool APReceiveConfigurationUpdateRequest() 
{
	char buf[AP_BUFFER_SIZE];
	APNetworkAddress addr;
	int readBytes;
	u32 recvAddr;
	
	/* receive the datagram */
	if(!APErr(APNetworkReceive(buf,
					 AP_BUFFER_SIZE - 1,
                     &addr,
					 &readBytes))) {
        APErrorLog("Receive Configuration Update Request failed");
		return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
	}

	recvAddr = ntohl(addr.sin_addr.s_addr);
    /* verify the source of the message */
    if(recvAddr != gControllerIPAddr) {
        APErrorLog("Message from the illegal source address");
        return APErrorRaise(AP_ERROR_WARNING, "APReceiveConfigurationUpdateRequest()");
    }
	
	/* check if it is a valid Configuration Update Request */
	if(!APErr(APParseConfigurationUpdateRequest(buf, readBytes))) {
		return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
	}

	APDebugLog(3, "Accept Configuration Update Request");
	
	return AP_TRUE;
}

APBool APReadConfigurationUpdateRequest()
{

	struct timeval timeout, new_timeout, before, after, delta;
	
	timeout.tv_sec = new_timeout.tv_sec = gConfigureInterval;
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
                if(APErr(APReceiveConfigurationUpdateRequest())) {
                    return AP_TRUE;
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
        APDebugLog(3, "Timer expired during read configuration update request");
	
	APLog("There is no valid Configuration Update Request");
    return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
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

        gConfigureCount++;
        APDebugLog(3, "The number of Configure operations = %d", gConfigureCount);

        /* wait for Configuration Update Request */
        if(!APErr(APReadConfigurationUpdateRequest())) {
			continue; // no conf update req or invalid conf update req
		}

        /* generate the hostapd configuration file */
        HdGenerateConfigurationFile();
        APDebugLog(3, "Generate the configuration file successfully");
        APLog("Start hostapd...");
        exit(1);
    }
}