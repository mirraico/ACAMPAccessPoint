#include "common.h"
#include "protocol.h"

APBool APAssembleDiscoveryResponse(APProtocolMessage *messagesPtr, int seq)
{
    int k = -1;
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 4;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return AP_FALSE;);

    if(
	   (!(APAssembleControllerName(&(msgElems[++k])))) ||
	   (!(APAssembleControllerDescriptor(&(msgElems[++k])))) ||
	   (!(APAssembleControllerIPAddr(&(msgElems[++k])))) ||
	   (!(APAssembleControllerMACAddr(&(msgElems[++k]))))
	   )
	{
		int i;
		for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		AP_FREE_OBJECT(msgElems);
		return AP_FALSE;
	}
	
	return APAssembleControlMessage(messagesPtr, 
				 0,
				 seq,
				 MSGTYPE_DISCOVERY_RESPONSE,
				 msgElems,
				 msgElemCount
	);
}

APBool APAssembleRegisterResponse(APProtocolMessage *messagesPtr, int seq)
{
    int k = -1;
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 7;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return AP_FALSE;);

    if(
	   (!(APAssembleControllerName(&(msgElems[++k])))) ||
	   (!(APAssembleControllerDescriptor(&(msgElems[++k])))) ||
	   (!(APAssembleControllerIPAddr(&(msgElems[++k])))) ||
	   (!(APAssembleControllerMACAddr(&(msgElems[++k])))) ||
       (!(APAssembleResultCode(&(msgElems[++k])))) ||
       (!(APAssembleAssignedAPID(&(msgElems[++k])))) ||
       (!(APAssembleRegisteredService(&(msgElems[++k]))))
	   )
	{
		int i;
		for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		AP_FREE_OBJECT(msgElems);
		return AP_FALSE;
	}
	
	return APAssembleControlMessage(messagesPtr, 
				 0,
				 seq,
				 MSGTYPE_REGISTER_RESPONSE,
				 msgElems,
				 msgElemCount
	);
}

APBool APAssembleRegisterReject(APProtocolMessage *messagesPtr, int seq)
{
    int k = -1;
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 2;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return AP_FALSE;);

    if(
       (!(APAssembleReasonCode(&(msgElems[++k])))) ||
       (!(APAssembleResultReject(&(msgElems[++k]))))
	   )
	{
		int i;
		for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		AP_FREE_OBJECT(msgElems);
		return AP_FALSE;
	}
	
	return APAssembleControlMessage(messagesPtr, 
				 0,
				 seq,
				 MSGTYPE_REGISTER_RESPONSE,
				 msgElems,
				 msgElemCount
	);
}

APBool APAssembleConfigurationRequest(APProtocolMessage *messagesPtr, int seq)
{
    int k = -1;
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0;
	AP_CREATE_PROTOCOL_ARRAY(msgElems, msgElemCount, return AP_FALSE;);
	
	return APAssembleControlMessage(messagesPtr, 
				 0,
				 seq,
				 MSGTYPE_CONFIGURATION_UPDATE_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

int main()
{
	struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6606);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int sock;
    if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        exit(1);
    }
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

	char buff[512];
    struct sockaddr_in clientAddr;
    int n;
    int len = sizeof(clientAddr);
    int process = 0;
    while (1)
    {
        n = recvfrom(sock, buff, 511, 0, (struct sockaddr*)&clientAddr, &len);
        if (n>0)
        {
            buff[n] = 0;
            printf("recv from %s %u\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        }
        else
        {
            perror("recvfrom");
            return 0;
        }
        switch(process) {
            case 0: //discovery
                {
                    APHeaderVal controlVal;
                    APProtocolMessage completeMsg;
                    completeMsg.msg = buff;
                    completeMsg.offset = 0;
                    if(!(APParseControlHeader(&completeMsg, &controlVal))) {
                        return AP_FALSE;
                    }
                    int seq = controlVal.seqNum;
                    APProtocolMessage sendMsg;
                    AP_INIT_PROTOCOL(sendMsg);
                    APAssembleDiscoveryResponse(&sendMsg, seq);
                    n = sendto(sock, sendMsg.msg, sendMsg.offset, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
                    if (n < 0)
                    {
                        perror("sendto");
                        return 0;
                    }
                    AP_FREE_PROTOCOL_MESSAGE(sendMsg);
                }
                break;
            case 1: //register
                {
                    APHeaderVal controlVal;
                    APProtocolMessage completeMsg;
                    completeMsg.msg = buff;
                    completeMsg.offset = 0;
                    if(!(APParseControlHeader(&completeMsg, &controlVal))) {
                        return AP_FALSE;
                    }
                    int seq = controlVal.seqNum;
                    APProtocolMessage sendMsg;
                    AP_INIT_PROTOCOL(sendMsg);
                    APAssembleRegisterResponse(&sendMsg, seq);
                    // APAssembleRegisterReject(&sendMsg, seq);
                    n = sendto(sock, sendMsg.msg, sendMsg.offset, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
                    if (n < 0)
                    {
                        perror("sendto");
                        return 0;
                    }
                    AP_FREE_PROTOCOL_MESSAGE(sendMsg);
                }
            break;
            case 2: //configure
                {
                    APHeaderVal controlVal;
                    APProtocolMessage completeMsg;
                    completeMsg.msg = buff;
                    completeMsg.offset = 0;
                    if(!(APParseControlHeader(&completeMsg, &controlVal))) {
                        return AP_FALSE;
                    }
                    int seq = controlVal.seqNum;
                    APProtocolMessage sendMsg;
                    AP_INIT_PROTOCOL(sendMsg);
                    APAssembleConfigurationRequest(&sendMsg, 164613215);
                    n = sendto(sock, sendMsg.msg, sendMsg.offset, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
                    if (n < 0)
                    {
                        perror("sendto");
                        return 0;
                    }
                    AP_FREE_PROTOCOL_MESSAGE(sendMsg);
                }
            break;

            default:
            break;
        }
        process = (process + 1) % 3;
    }
    return 0;
}
