#include "Common.h"
#include "AcampAP.h"
#include "AcampNetwork.h"
#include "AcampProtocol.h"

void APInitConfiguration();

APBool APParseDiscoveryResponseMessage(u8 *msg, 
				       int msgLen,
					   u32 *seqNumPtr) 
{
	if(msg == NULL || seqNumPtr == NULL) 
		return AP_FALSE;
	APHeaderVal header;
	APProtocolMessage completeMsg;
	completeMsg.msg = (u8*)msg;
	completeMsg.offset = 0;
	
	if(!APParseControlHeader(&completeMsg, &header)) return AP_FALSE;
	if(header.msgType != MSGTYPE_DISCOVER_RESPONSE)
		return AP_FALSE;
	*seqNumPtr = header.seqNum;
	
	while((msgLen - completeMsg.offset) > 0) {
		u16 elemType = 0;
		u16 elemLen = 0;
		if(!APParseFormatMsgElement(&completeMsg, &elemType, &elemLen)) {
			return AP_FALSE;
		}
		switch(elemType) {
			case MSGELETYPE_CONTROLLER_NAME:
				if(!APParseMsgElemControllerName(&completeMsg, elemLen))
					return AP_FALSE;
				break;
			case MSGELETYPE_CONTROLLER_DESCRIPTOR:
				if(!APParseMsgElemControllerDescriptor(&completeMsg, elemLen))
					return AP_FALSE;
				break; 
			case MSGELETYPE_CONTROLLER_IP_ADDRESS:
				if(!APParseMsgElemControllerIPAddr(&completeMsg, elemLen))
					return AP_FALSE;
				break;
			case MSGELETYPE_CONTROLLER_MAC_ADDRESS:
				if(!APParseMsgElemControllerMACAddr(&completeMsg, elemLen))
					return AP_FALSE;
				break;
			default:
				return AP_FALSE;
				break;
		}
	}
	if(msgLen != completeMsg.offset) return AP_FALSE;
	
	return AP_TRUE;
}

APBool APReceiveDiscoveryResponse()
{
	u8 buffer[AP_BUFFER_SIZE];
	int readBytes; APNetworkAddress controllerAddr;
	if(!APNetworkReceiveUnconnected(buffer, AP_BUFFER_SIZE - 1, &readBytes, &controllerAddr)) {
		return AP_FALSE;
	}
	u32 seqNum;
	if(!APParseDiscoveryResponseMessage(buffer, readBytes, &seqNum)) {
		return AP_FALSE;
	}
	return AP_TRUE;
}

APBool interactiveTestDiscoveryRequest(APProtocolMessage *messagesPtr) 
{
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0, k = 0;
	AP_CREATE_PROTOCOL_ARRAY_AND_INIT(msgElems,  msgElemCount);
	
	return APAssembleMessage (messagesPtr, 
				 15432,
				 MSGTYPE_DISCOVER_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

APBool interactiveTestRegisterRequest(APProtocolMessage *messagesPtr) 
{
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 4, k = 0;
	AP_CREATE_PROTOCOL_ARRAY_AND_INIT(msgElems,  msgElemCount);
	
	if(
		(!(APAssembleMsgElemAPName(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemAPDescriptor(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemAPIPAddr(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemAPMACAddr(&(msgElems[k++]))))
		)
	{
		int i;
		for(i = 0; i < k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		AP_FREE_OBJECT(msgElems);
		return AP_FALSE;
	}
	
	return APAssembleMessage (messagesPtr, 
				 15433,
				 MSGTYPE_REGISTER_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

APBool interactiveTestConfigurationRequest(APProtocolMessage *messagesPtr) 
{
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 1, k = 0;
	AP_CREATE_PROTOCOL_ARRAY_AND_INIT(msgElems,  msgElemCount);
	
	if(
	   (!(APAssembleMsgElemAPDescriptor(&(msgElems[k++]))))	
	   )
	{
		int i;
		for(i = 0; i < k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		AP_FREE_OBJECT(msgElems);
		return AP_FALSE;
	}
	
	return APAssembleMessage (messagesPtr, 
				 15432,
				 MSGTYPE_CONFIGURATION_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

APBool interactiveTestConfigurationReport(APProtocolMessage *messagesPtr) 
{
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 1, k = 0;
	AP_CREATE_PROTOCOL_ARRAY_AND_INIT(msgElems,  msgElemCount);
	
	if(
	   (!(APAssembleMsgElemAPDescriptor(&(msgElems[k++]))))	
	   )
	{
		int i;
		for(i = 0; i < k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		AP_FREE_OBJECT(msgElems);
		return AP_FALSE;
	}
	
	return APAssembleMessage (messagesPtr, 
				 15432,
				 MSGTYPE_CONFIGURATION_REPORT,
				 msgElems,
				 msgElemCount
	);
}

void interactiveTest()
{
	APNetworkInit();
	APNetworkInitLocalAddr();
	APInitConfiguration();
	
	APProtocolMessage sendMsg;
	AP_INIT_PROTOCOL_MESSAGE(sendMsg);
	interactiveTestDiscoveryRequest(&sendMsg);
	APNetworkSendToBroadUnconnected(sendMsg);
	AP_FREE_PROTOCOL_MESSAGE(sendMsg);
	
	APReceiveDiscoveryResponse();
	//strcpy(gControllerAddr, "192.168.1.1\0");
	APNetworkInitControllerAddr();
	
	AP_INIT_PROTOCOL_MESSAGE(sendMsg);
	interactiveTestRegisterRequest(&sendMsg);
	APNetworkSendUnconnected(sendMsg);
	AP_FREE_PROTOCOL_MESSAGE(sendMsg);
//	/*
//	recvfrom(gSocket, buffer, 1024, 0, (struct sockaddr*)&controllerSockAddr, &controllerSockAddrLen) ;
//	APNetworkInitControllerAddr(controllerSockAddr);
//	*/
//	AP_INIT_PROTOCOL_MESSAGE(sendMsg);
//	interactiveTestConfigurationRequest(&sendMsg);
//	APNetworkSendUnconnected(sendMsg);
//	AP_FREE_PROTOCOL_MESSAGE(sendMsg);
//	/*
//	recvfrom(gSocket, buffer, 1024, 0, (struct sockaddr*)&controllerSockAddr, &controllerSockAddrLen) ;
//	APNetworkInitControllerAddr(controllerSockAddr);
//	*/
//	AP_INIT_PROTOCOL_MESSAGE(sendMsg);
//	interactiveTestConfigurationReport(&sendMsg);
//	APNetworkSendUnconnected(sendMsg);
//	AP_FREE_PROTOCOL_MESSAGE(sendMsg);
}

int main()
{
//	APNetworkInitLocalAddr();
//	printf("IP:  %u.%u.%u.%u\n", (u8)(gAPIPAddr >> 24), (u8)(gAPIPAddr >> 16),  (u8)(gAPIPAddr >> 8),  (u8)(gAPIPAddr >> 0));
//	printf("MAC:  %u:%u:%u:%u:%u:%u\n", gAPMacAddr[0], gAPMacAddr[1], gAPMacAddr[2], gAPMacAddr[3], gAPMacAddr[4], gAPMacAddr[5]);
//	printf("IP:  %u.%u.%u.%u\n", (u8)(gAPDefaultGateway >> 24), (u8)(gAPDefaultGateway >> 16),  (u8)(gAPDefaultGateway >> 8),  (u8)(gAPDefaultGateway >> 0));
	
	interactiveTest();
	/*
	APInitConfiguration();
	APNetworkInitLocalAddr();
	
	APStateTransition nextState = AP_DISCOVERY;
	//AP_REPEAT_FOREVER
	{
		switch(nextState)
		{
			case AP_DISCOVERY:
				nextState = APEnterDiscovery();
				break;
			case AP_REGISTER:
				break;
			case AP_RUN:
				break;
			case AP_DOWN:
				break;
		}
	}
	*/

	return 0;
}

void APInitConfiguration()
{
	strcpy(gAPName, "AP-NAME\0");
	strcpy(gAPDescriptor, "AP-DESCRIPTOR\0");
}

void APTest()
{
	/*
	 Local NetworkInfo Test 
	 printf("ip: %s gateway: %s\n", gLocalAddr, gLocalDefaultGateway);
	 */
	 
	/*
	Send Test
	
	APProtocolMessage *msgElems;
	int msgElemCount = 1;
	AP_CREATE_PROTOCOL_ARRAY_AND_INIT(msgElems,  msgElemCount);
	APAssembleMsgElemAPName(&(msgElems[0]));
	APProtocolMessage sendMsg;
	AP_INIT_PROTOCOL_MESSAGE(sendMsg);
	APAssembleMessage(&sendMsg, 12345, MSGTYPE_CONFIGURATION_REQUEST, msgElems, msgElemCount);
	APNetworkSendMessage(sendMsg.msg, sendMsg.offset);
	AP_FREE_PROTOCOL_MESSAGE(sendMsg);
	*/
}