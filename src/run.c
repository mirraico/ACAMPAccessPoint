#include "common.h"
#include "error.h"
#include "log.h"
#include "ap.h"
#include "network.h"

#define MAX_EVENT_NUMBER 1024

struct uloop_fd fdSocket;
struct uloop_timeout tKeepAlive;
struct uloop_timeout tRetransmit;
APProtocolMessage retransmitMsg;
int retransmitInterval;
int retransmitCount;
APProtocolMessage cacheMsg;

static void APRretransmitHandler(struct uloop_timeout *t)
{
    retransmitCount++;
    if(retransmitCount >= 5) {
        APLog("There is no valid Response for %d times", retransmitCount);
        APLog("The connection to controller has been interrupted");
        uloop_end();
        return;
    }

    APLog("There is no valid Response, times = %d, retransmit request", retransmitCount);
    retransmitInterval *= 2;
    if(retransmitInterval > gKeepAliveInterval / 2) {
        retransmitInterval = gKeepAliveInterval / 2;
    }
    uloop_timeout_set(&tRetransmit, retransmitInterval * 1000);
    APDebugLog(5, "Adjust the retransmit interval to %d sec and retransmit", retransmitInterval);
    if(!APErr(APNetworkSend(retransmitMsg))) {
        APErrorLog("Failed to retransmit Request");
        uloop_end();
        return;
    }
}

APBool APAssembleKeepAliveRequest(APProtocolMessage *messagesPtr)
{
    int k = -1;
	if(messagesPtr == NULL) APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleKeepAliveRequest()");
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleKeepAliveRequest()"););
	
	return APAssembleControlMessage(messagesPtr, 
				 APGetAPID(),
				 APGetSeqNum(),
				 MSGTYPE_KEEPALIVE_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

static void APKeepAliveHandler(struct uloop_timeout *t) 
{
    APProtocolMessage sendMsg;
    AP_INIT_PROTOCOL(sendMsg);
    if(!APErr(APAssembleKeepAliveRequest(&sendMsg))) {
        APErrorLog("Failed to assemble Keep Alive Request");
        uloop_end();
        return;
    }
    APLog("Send Keep Alive Request");
    if(!APErr(APNetworkSend(sendMsg))) {
        APErrorLog("Failed to send Keep Alive Request");
        uloop_end();
        return;
    }

    AP_INIT_PROTOCOL_MESSAGE(retransmitMsg, sendMsg.offset, APErrorLog("Failed to init Retransmit Message"); uloop_end(); return;);
    APProtocolStoreMessage(&retransmitMsg, &sendMsg);
    retransmitMsg.type = MSGTYPE_KEEPALIVE_REQUEST; //easy to match response
    AP_FREE_PROTOCOL_MESSAGE(sendMsg);
    
    retransmitInterval = 3;
    retransmitCount = 0;
    uloop_timeout_set(&tRetransmit, retransmitInterval * 1000);
}

APBool APParseKeepAliveResponse()
{
    uloop_timeout_cancel(&tKeepAlive);
    uloop_timeout_set(&tKeepAlive, gKeepAliveInterval * 1000);
    APLog("Accept Keep Alive Response");
    return AP_TRUE;
}

APBool APAssembleConfigurationUpdateResponse(APProtocolMessage *messagesPtr)
{
	if(messagesPtr == NULL) APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleConfigurationResponse()");
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleConfigurationResponse()"););
	
	return APAssembleControlMessage(messagesPtr, 
				 APGetAPID(),
				 APGetControllerSeqNum(),
				 MSGTYPE_CONFIGURATION_UPDATE_RESPONSE,
				 msgElems,
				 msgElemCount
	);
}

APBool APParseConfigurationUpdateRequest(APProtocolMessage *completeMsg, u16 msgLen)
{
    APLog("Parse Configuration Update Request");

    u16 elemFlag = 0;

    /* parse message elements */
	while(completeMsg->offset < msgLen) 
    {
		u16 type = 0;
		u16 len = 0;

		APParseFormatMsgElem(completeMsg, &type, &len);
	    // APDebugLog(3, "Parsing Message Element: %u, len: %u", type, len);
        switch(type) 
        {
            case MSGELEMTYPE_SSID:
            {
                char *ssid;
                if(!(APParseSSID(completeMsg, len, &ssid)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                wlconf->set_ssid(wlconf, ssid);
                AP_FREE_OBJECT(ssid);
				break;
            }
            case MSGELEMTYPE_CHANNEL:
            {
                u8 channel;
                if(!(APParseChannel(completeMsg, len, &channel)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                wlconf->set_channel(wlconf, channel);
				break;
            }
            case MSGELEMTYPE_HARDWARE_MODE:
            {
                u8 hw_mode;
                if(!(APParseHardwareMode(completeMsg, len, &hw_mode)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                switch(hw_mode)
                {
                    case HWMODE_A:
                        wlconf->set_hwmode(wlconf, ONLY_A);
                        break;
                    case HWMODE_B:
                        wlconf->set_hwmode(wlconf, ONLY_B);
                        break;
                    case HWMODE_G:
                        wlconf->set_hwmode(wlconf, ONLY_G);
                        break;
                    case HWMODE_N:
                        wlconf->set_hwmode(wlconf, ONLY_N);
                        break;
                    default:
                        break;
                }
				break;
            }
            case MSGELEMTYPE_SUPPRESS_SSID:
            {
                u8 suppress;
                if(!(APParseSuppressSSID(completeMsg, len, &suppress)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                if(suppress == SUPPRESS_SSID_DISABLED) wlconf->set_ssid_hidden(wlconf, false);
                else wlconf->set_ssid_hidden(wlconf, true);
				break;
            }
            case MSGELEMTYPE_SECURITY_OPTION:
            {
                u8 security;
                if(!(APParseSecurityOption(completeMsg, len, &security)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                switch(security)
                {
                    case SECURITY_OPEN:
                        wlconf->set_encryption(wlconf, NO_ENCRYPTION);
                        break;
                    case SECURITY_WPA_WPA2_MIXED:
                        wlconf->set_encryption(wlconf, WPA_WPA2_MIXED);
                        break;
                    case SECURITY_WPA2:
                        wlconf->set_encryption(wlconf, WPA2_PSK);
                        break;
                    case SECURITY_WPA:
                        wlconf->set_encryption(wlconf, WPA_PSK);
                        break;
                    default:
                        break;
                }
				break;
            }
            case MSGELEMTYPE_MACFILTER_MODE:
            {
                u8 mode;
                if(!(APParseMACFilterMode(completeMsg, len, &mode)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                switch(mode)
                {
                    case FILTER_NONE:
                        wlconf->set_macfilter(wlconf, MAC_FILTER_NONE);
                        break;
                    case FILTER_ACCEPT_ONLY:
                        wlconf->set_macfilter(wlconf, MAC_FILTER_ALLOW);
                        break;
                    case FILTER_DENY:
                        wlconf->set_macfilter(wlconf, MAC_FILTER_DENY);
                        break;
                    default:
                        break;
                }
				break;
            }
            case MSGELEMTYPE_TX_POWER:
            {
                u8 tx_power;
                if(!(APParseTxPower(completeMsg, len, &tx_power)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                wlconf->set_txpower(wlconf, tx_power);
				break;
            }
            case MSGELEMTYPE_WPA_PWD:
            {
                char *pwd;
                if(!(APParseWPAPassword(completeMsg, len, &pwd)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                wlconf->set_key(wlconf, pwd);
                AP_FREE_OBJECT(pwd);
				break;
            }
            case MSGELEMTYPE_ADD_MACFILTER:
            {
                int i;
                if(len % 6) {
                    APErrorLog("Message Element Malformed in MAC Addr");
                    break;
                }
                int num = len / 6;
                char **maclist;
                AP_CREATE_ARRAY_ERR(maclist, len, char*, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APParseMACList()");)
                if(!(APParseMACList(completeMsg, len, &maclist)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                for(i = 0; i < num; i++) {
                    wlconf->add_macfilterlist(wlconf, maclist[i]);
                    APDebugLog(5, "Add MAC Filter List: %s", maclist[i]);
                    AP_FREE_OBJECT(maclist[i]);
                }
                AP_FREE_OBJECT(maclist);
				break;
            }
            case MSGELEMTYPE_DEL_MACFILTER:
            {
                int i;
                if(len % 6) {
                    APErrorLog("Message Element Malformed in MAC Addr");
                    break;
                }
                int num = len / 6;
                char **maclist;
                AP_CREATE_ARRAY_ERR(maclist, len, char*, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APParseMACList()");)
                if(!(APParseMACList(completeMsg, len, &maclist)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                for(i = 0; i < num; i++) {
                    wlconf->del_macfilterlist(wlconf, maclist[i]);
                    APDebugLog(5, "Delete MAC Filter List: %s", maclist[i]);
                    AP_FREE_OBJECT(maclist[i]);
                }
                AP_FREE_OBJECT(maclist);
				break;
            }
            case MSGELEMTYPE_CLEAR_MACFILTER:
            {
                wlconf->clear_macfilterlist(wlconf);
                APDebugLog(5, "Clear MAC Filter List");
				break;
            }
            case MSGELEMTYPE_RESET_MACFILTER:
            {
                int i;
                if(len % 6) {
                    APErrorLog("Message Element Malformed in MAC Addr");
                    break;
                }
                int num = len / 6;
                char **maclist;
                AP_CREATE_ARRAY_ERR(maclist, len, char*, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APParseMACList()");)
                if(!(APParseMACList(completeMsg, len, &maclist)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                wlconf->clear_macfilterlist(wlconf);
                for(i = 0; i < num; i++) {
                    wlconf->add_macfilterlist(wlconf, maclist[i]);
                    APDebugLog(5, "Reset & Add MAC Filter List: %s", maclist[i]);
                    AP_FREE_OBJECT(maclist[i]);
                }
                AP_FREE_OBJECT(maclist);
				break;
            }
            default:
                APParseUnrecognizedMsgElem(completeMsg, len);
                APErrorLog("Unrecognized Message Element");
				break;
        }
    }

    wlconf->change_commit(wlconf);
    system("wifi restart");
    APLog("Accept Configuration Update Request");
    return AP_TRUE;
}

APBool APAssembleConfigurationResponse(APProtocolMessage *messagesPtr, u8** listPtr, int listSize)
{
    int k = -1;
    int pos = 0;
    u16 desiredType = 0;
    u8* list = *listPtr;
	if(messagesPtr == NULL) APErrorRaise(AP_ERROR_WRONG_ARG, "APAssembleConfigurationResponse()");
	
	APProtocolMessage *msgElems;
	int msgElemCount = listSize;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APAssembleConfigurationResponse()"););

    wlconf->update(wlconf);
    while(pos < listSize * 2)
    {
        AP_COPY_MEMORY(&desiredType, list + pos, 2);

        switch(desiredType)
        {
            case MSGELEMTYPE_SSID:
                if(!(APAssembleSSID(&(msgElems[++k])))) {
                    int i;
                    for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
                    AP_FREE_OBJECT(msgElems);
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                }
                break;
            case MSGELEMTYPE_CHANNEL:
                if(!(APAssembleChannel(&(msgElems[++k])))) {
                    int i;
                    for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
                    AP_FREE_OBJECT(msgElems);
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                }
                break;
            case MSGELEMTYPE_HARDWARE_MODE:
                if(!(APAssembleHardwareMode(&(msgElems[++k])))) {
                    int i;
                    for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
                    AP_FREE_OBJECT(msgElems);
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                }
                break;
            case MSGELEMTYPE_SUPPRESS_SSID:
                if(!(APAssembleSuppressSSID(&(msgElems[++k])))) {
                    int i;
                    for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
                    AP_FREE_OBJECT(msgElems);
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                }
                break;
            case MSGELEMTYPE_SECURITY_OPTION:
                if(!(APAssembleSecurityOption(&(msgElems[++k])))) {
                    int i;
                    for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
                    AP_FREE_OBJECT(msgElems);
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                }
                break;
            case MSGELEMTYPE_MACFILTER_MODE:
                if(!(APAssembleMACFilterMode(&(msgElems[++k])))) {
                    int i;
                    for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
                    AP_FREE_OBJECT(msgElems);
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                }
                break;
            case MSGELEMTYPE_MACFILTER_LIST:
                if(!(APAssembleMACFilterList(&(msgElems[++k])))) {
                    int i;
                    for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
                    AP_FREE_OBJECT(msgElems);
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                }
                break;
            case MSGELEMTYPE_TX_POWER:
                if(!(APAssembleTxPower(&(msgElems[++k])))) {
                    int i;
                    for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
                    AP_FREE_OBJECT(msgElems);
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                }
                break;
            case MSGELEMTYPE_WPA_PWD:
                if(!(APAssembleWPAPassword(&(msgElems[++k])))) {
                    int i;
                    for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
                    AP_FREE_OBJECT(msgElems);
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
                }
                break;
            default:
                APErrorLog("Unknown desired configuration type");
                break;
        }

        pos += 2;
    }

    AP_FREE_OBJECT(list);
	
	return APAssembleControlMessage(messagesPtr, 
				 APGetAPID(),
				 APGetControllerSeqNum(),
				 MSGTYPE_CONFIGURATION_RESPONSE,
				 msgElems,
				 msgElemCount
	);
}

APBool APParseConfigurationRequest(APProtocolMessage *completeMsg, u16 msgLen, u8 **listPtr, int *listSize)
{
    APLog("Parse Configuration Request");

    u16 elemFlag = 0;

    /* parse message elements */
	while(completeMsg->offset < msgLen) 
    {
		u16 type = 0;
		u16 len = 0;

		APParseFormatMsgElem(completeMsg, &type, &len);
	    // APDebugLog(3, "Parsing Message Element: %u, len: %u", type, len);
        switch(type) 
        {
            case MSGELEMTYPE_DESIRED_CONF_LIST:
                if(elemFlag & 0x01) {
                    APParseRepeatedMsgElem(completeMsg, len);
                    APErrorLog("Repeated Message Element");
                    break;
                }
                *listSize = len / 2; //each type takes 2 bytes
                APDebugLog(5, "Parse Desired Conf List Size: %d", *listSize);

                if(!(APParseDesiredConfList(completeMsg, len, listPtr)))
                    return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);

                elemFlag |= 0x01;
				break;
            default:
                APParseUnrecognizedMsgElem(completeMsg, len);
                APErrorLog("Unrecognized Message Element");
				break;
        }
    }
    if(elemFlag != 0x01) { //incomplete message
        APErrorLog("Incomplete Message Element in Configuration Request");
        return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseConfigurationRequest()");
    }

    if(*listSize == 0) {
        APErrorLog("There is no requested configuration");
    }
    APLog("Accept Configuration Request");

    return AP_TRUE;
}

APBool APReceiveMessageInRunState() 
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
        APErrorLog("Receive Message in run state failed");
		return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
	}

    recvAddr = ntohl(addr.sin_addr.s_addr);
    /* verify the source of the message */
    if(recvAddr != gControllerIPAddr) {
        APErrorLog("Message from the illegal source address");
        return APErrorRaise(AP_ERROR_WARNING, "APReceiveMessageInRunState()");
    }

    APDebugLog(3, "Receive Message in run state");

    APHeaderVal controlVal;
	APProtocolMessage completeMsg;
    completeMsg.msg = buf;
	completeMsg.offset = 0;

    if(!(APParseControlHeader(&completeMsg, &controlVal))) {
        APErrorLog("Failed to parse header");
        return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
    }

    /* not as expected */
    if(controlVal.version != CURRENT_VERSION || controlVal.type != TYPE_CONTROL) {
        APErrorLog("ACAMP version or type is not Expected");
        return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APReceiveMessageInRunState()");
    }
    if(controlVal.apid != gAPID) {
        APErrorLog("The apid in message is different from the one in message header");
        return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
    }

    int is_req = controlVal.msgType % 2; //odd type indicates the request message

    if(is_req) {
        if(controlVal.seqNum == APGetControllerSeqNum() - 1) {
            APDebugLog(3, "Receive a message that has been responsed");
            if(cacheMsg.type == 0 || controlVal.msgType + 1 != cacheMsg.type) {
                APErrorLog("The received message does not match the cache message");
                return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
            }

            APDebugLog(3, "Send Cache Message");
            if(!APErr(APNetworkSend(cacheMsg))) {
                APErrorLog("Failed to send Cache Message");
                return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
            }

            uloop_timeout_cancel(&tKeepAlive);
            uloop_timeout_set(&tKeepAlive, gKeepAliveInterval * 1000);
            return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
        }
        if(controlVal.seqNum != APGetControllerSeqNum()) {
            if(controlVal.seqNum < APGetControllerSeqNum())
                APErrorLog("The serial number of the message is expired");
            else
                APErrorLog("The serial number of the message is invalid");
            return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
        }

        switch(controlVal.msgType)
        {
            case MSGTYPE_CONFIGURATION_REQUEST:
            {
                u8* list = NULL;
                int listSize; 
                if(!APErr(APParseConfigurationRequest(&completeMsg, controlVal.msgLen, &list, &listSize))) {
                    return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
                }

                APProtocolMessage responseMsg;
                AP_INIT_PROTOCOL(responseMsg);
                if(!APErr(APAssembleConfigurationResponse(&responseMsg, &list, listSize))) {
                    APErrorLog("Failed to assemble Configuration Response");
                    return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
                }
                APLog("Send Configuration Response");
                if(!APErr(APNetworkSend(responseMsg))) {
                    APErrorLog("Failed to send Configuration Response");
                    return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
                }
                AP_FREE_OBJECT(list);

                AP_FREE_PROTOCOL_MESSAGE(cacheMsg);
                AP_INIT_PROTOCOL_MESSAGE(cacheMsg, responseMsg.offset, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APReceiveMessageInRunState()"););
                APProtocolStoreMessage(&cacheMsg, &responseMsg);
                cacheMsg.type = MSGTYPE_CONFIGURATION_RESPONSE; //easy to match request
                AP_FREE_PROTOCOL_MESSAGE(responseMsg);

                APControllerSeqNumIncrement();
                uloop_timeout_cancel(&tKeepAlive);
                uloop_timeout_set(&tKeepAlive, gKeepAliveInterval * 1000);
                break;
            }
            case MSGTYPE_CONFIGURATION_UPDATE_REQUEST:
                if(!APErr(APParseConfigurationUpdateRequest(&completeMsg, controlVal.msgLen))) {
                    return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
                }

                APProtocolMessage responseMsg;
                AP_INIT_PROTOCOL(responseMsg);
                if(!APErr(APAssembleConfigurationUpdateResponse(&responseMsg))) {
                    APErrorLog("Failed to assemble Configuration Update Response");
                    return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
                }
                APLog("Send Configuration Update Response");
                if(!APErr(APNetworkSend(responseMsg))) {
                    APErrorLog("Failed to send Configuration Update Response");
                    return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
                }

                AP_FREE_PROTOCOL_MESSAGE(cacheMsg);
                AP_INIT_PROTOCOL_MESSAGE(cacheMsg, responseMsg.offset, return APErrorRaise(AP_ERROR_OUT_OF_MEMORY, "APReceiveMessageInRunState()"););
                APProtocolStoreMessage(&cacheMsg, &responseMsg);
                cacheMsg.type = MSGTYPE_CONFIGURATION_UPDATE_RESPONSE; //easy to match request
                AP_FREE_PROTOCOL_MESSAGE(responseMsg);

                APControllerSeqNumIncrement();
                uloop_timeout_cancel(&tKeepAlive);
                uloop_timeout_set(&tKeepAlive, gKeepAliveInterval * 1000);
                break;
            case MSGTYPE_SYSTEM_REQUEST:
                //TODO: 
                break;
            case MSGTYPE_UNREGISTER_REQUEST:
                //TODO: 
                break;
            default:
                return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
        }


    } else {
        if(controlVal.seqNum != APGetSeqNum()) {
            if(controlVal.seqNum < APGetSeqNum())
                APErrorLog("The serial number of the message is expired");
            else
                APErrorLog("The serial number of the message is invalid");
            return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
        }
        if(retransmitMsg.type == 0 || controlVal.msgType != retransmitMsg.type + 1) {
            APErrorLog("The received message does not match the send message");
            return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
        }
        switch(controlVal.msgType)
        {
            case MSGTYPE_KEEPALIVE_RESPONSE:
                if(!APErr(APParseKeepAliveResponse())) {
                    return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
                }
                uloop_timeout_cancel(&tRetransmit);
                AP_FREE_PROTOCOL_MESSAGE(retransmitMsg);
                APSeqNumIncrement();
                break;
            default:
                return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
        }
    }

    return AP_TRUE;
}

static void APEvents(struct uloop_fd *event_fd, unsigned int events)
{
    int sockfd = event_fd->fd;
    if(sockfd != gSocket) {
        APErrorLog("Received an event from unknown source");
    }
    APErr(APReceiveMessageInRunState());
}

APStateTransition APEnterRun() 
{
    APLog("");	
	APLog("######### Run State #########");

    AP_FREE_PROTOCOL_MESSAGE(cacheMsg);
    AP_FREE_PROTOCOL_MESSAGE(retransmitMsg);


    uloop_init();

    tRetransmit.cb = APRretransmitHandler;
	tKeepAlive.cb = APKeepAliveHandler;
    uloop_timeout_set(&tKeepAlive, 100); //send a keep alive req soon

    fdSocket.fd = gSocket;
    fdSocket.cb = APEvents;
    uloop_fd_add(&fdSocket, ULOOP_READ);

	uloop_run();

    uloop_done();

	return AP_ENTER_DOWN;
}