#include "common.h"
#include "error.h"
#include "log.h"
#include "ap.h"
#include "network.h"
#include "hostapd.h"

#define MAX_EVENT_NUMBER 1024

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
        return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseConfigurationUpdateRequest()");
    }
    if(controlVal.apid != gAPID) {
        APErrorLog("The APID carried in the message is not this AP");
		return APErrorRaise(AP_ERROR_INVALID_FORMAT, "APParseConfigurationUpdateRequest()");
    }

    case(controlVal.msgType) {
        //TODO:
    }


    return AP_TRUE;
}

APStateTransition APEnterRun() 
{
    APLog("");	
	APLog("######### Run State #########");

    struct epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    if(epollfd == -1) {
        APErrorLog("EPOLL init failed");
        return AP_ENTER_DOWN;
    }
    APAddSocketToEpoll(epollfd, gSocket);
    APDebugLog(3, "Epoll Initialization successful");

    AP_REPEAT_FOREVER
    {
        int epoll_num = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        int i;
        if(epoll_num < 0)
        {
            APErrorLog("Epoll Runtime error");
            return AP_ENTER_DOWN;
        }
        for(i = 0; i < epoll_num; i++)
        {
            int sockfd = events[i].data.fd;
            if(sockfd == gSocket) {
                APErr(APReceiveMessageInRunState());
            }
        }
    }
}