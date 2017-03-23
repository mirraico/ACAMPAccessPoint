#include "common.h"
#include "error.h"
#include "log.h"
#include "ap.h"
#include "network.h"

#define MAX_EVENT_NUMBER 1024

struct uloop_fd fdSocket;
struct uloop_timeout tKeepAlive;
struct uloop_timeout tRretransmit;
APProtocolMessage retransmitMsg;
int retransmitInterval;
int retransmitCount;

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
    uloop_timeout_set(&tRretransmit, retransmitInterval * 1000);
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

    AP_INIT_PROTOCOL_MESSAGE(retransmitMsg, sendMsg.offset, APErrorLog("Failed to init Keep Alive Request"); uloop_end(); return;);
    APProtocolStoreMessage(&retransmitMsg, &sendMsg);
    retransmitMsg.type = MSGTYPE_KEEPALIVE_REQUEST; //easy to match response
    AP_FREE_PROTOCOL_MESSAGE(sendMsg);
    
    retransmitInterval = 3;
    retransmitCount = 0;
    uloop_timeout_set(&tRretransmit, retransmitInterval * 1000);
}

APBool APParseKeepAliveResponse()
{
    AP_FREE_PROTOCOL_MESSAGE(retransmitMsg);
    uloop_timeout_cancel(&tRretransmit);

    uloop_timeout_cancel(&tKeepAlive);
    uloop_timeout_set(&tKeepAlive, gKeepAliveInterval * 1000);
    APSeqNumIncrement();
    APLog("Accept Keep Alive Response");
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

    int is_req = controlVal.type % 2; //odd type indicates the request message

    if(is_req) {
        //TODO:
    } else {
        if(retransmitMsg.type == 0 || controlVal.msgType != retransmitMsg.type + 1) {
            APErrorLog("The type of message received is invalid");
            return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
        }
        if(controlVal.seqNum != APGetSeqNum()) {
            if(controlVal.seqNum < APGetSeqNum())
                APErrorLog("The serial number of the message is expired");
            else
                APErrorLog("The serial number of the message is invalid");
            return APErrorRaise(AP_ERROR_BUTNORAISE, NULL);
        }
        switch(controlVal.msgType)
        {
            case MSGTYPE_KEEPALIVE_RESPONSE:
                if(!APErr(APParseKeepAliveResponse())) {
                    return APErrorRaise(AP_ERROR_NOOUTPUT, NULL);
                }
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

    uloop_init();

    tRretransmit.cb = APRretransmitHandler;
	tKeepAlive.cb = APKeepAliveHandler;
    uloop_timeout_set(&tKeepAlive, 100); //send a keep alive req soon

    fdSocket.fd = gSocket;
    fdSocket.cb = APEvents;
    uloop_fd_add(&fdSocket, ULOOP_READ);

	uloop_run();

    uloop_done();

	return AP_ENTER_DOWN;

    // struct epoll_event events[MAX_EVENT_NUMBER];
    // int epollfd = epoll_create(5);
    // if(epollfd == -1) {
    //     APErrorLog("EPOLL init failed");
    //     return AP_ENTER_DOWN;
    // }
    // APAddSocketToEpoll(epollfd, gSocket);
    // APDebugLog(3, "Epoll Initialization successful");

    // AP_REPEAT_FOREVER
    // {
    //     int epoll_num = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
    //     int i;
    //     if(epoll_num < 0)
    //     {
    //         APErrorLog("Epoll Runtime error");
    //         return AP_ENTER_DOWN;
    //     }
    //     for(i = 0; i < epoll_num; i++)
    //     {
    //         int sockfd = events[i].data.fd;
    //         if(sockfd == gSocket) {
    //             APErr(APReceiveMessageInRunState());
    //         }
    //     }
    // }
}