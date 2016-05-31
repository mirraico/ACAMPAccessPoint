#include "Common.h"
#include "AcampAP.h"
#include "AcampNetwork.h"
#include "AcampProtocol.h"

void APInitConfiguration();

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
	   (!(APAssembleMsgElemAPDescriptor(&(msgElems[k++]))))	 ||
	   (!(APAssembleMsgElemAPIPAddress(&(msgElems[k++]))))
	   (!(APAssembleMsgElemAPMACAddress(&(msgElems[k++]))))
	   )
	{
		int i;
		for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		AP_FREE_OBJECT(msgElems);
		return AP_FALSE;
	}
	
	return APAssembleMessage (messagesPtr, 
				 15432,
				 MSGTYPE_REGISTER_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

APBool interactiveTestConfigurationRequest(APProtocolMessage *messagesPtr) 
{
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 3, k = 0;
	AP_CREATE_PROTOCOL_ARRAY_AND_INIT(msgElems,  msgElemCount);
	
	if(
	   (!(APAssembleMsgElemAPBoardData(&(msgElems[k++])))) ||
	   (!(APAssembleMsgElemAPDescriptor(&(msgElems[k++]))))	 ||
	   (!(APAssembleMsgElemAPRadioInformation(&(msgElems[k++]))))
	   )
	{
		int i;
		for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
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
	int msgElemCount = 3, k = 0;
	AP_CREATE_PROTOCOL_ARRAY_AND_INIT(msgElems,  msgElemCount);
	
	if(
	   (!(APAssembleMsgElemAPBoardData(&(msgElems[k++])))) ||
	   (!(APAssembleMsgElemAPDescriptor(&(msgElems[k++]))))	 ||
	   (!(APAssembleMsgElemAPRadioInformation(&(msgElems[k++]))))
	   )
	{
		int i;
		for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
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
	APProtocolMessage sendMsg;
	AP_INIT_PROTOCOL_MESSAGE(sendMsg);
	interactiveTestDiscoveryRequest(&sendMsg);
	APNetworkSendToBroadUnconnected(sendMsg.msg, sendMsg.offset);
	AP_FREE_PROTOCOL_MESSAGE(sendMsg);
	
	char buffer[1024]; 
	bzero(buffer, 1024); 
	struct sockaddr_in controllerSockAddr; unsigned int controllerSockAddrLen = sizeof(controllerSockAddr); 
	/*
	recvfrom(gSocket, buffer, 1024, 0, (struct sockaddr*)&controllerSockAddr, &controllerSockAddrLen) ;
	APNetworkInitControllerAddr(controllerSockAddr);
	*/
	strcpy(gControllerAddr,"192.168.1.1\0");
	APNetworkInitControllerAddr();
	
	APNetworkInit();
	AP_INIT_PROTOCOL_MESSAGE(sendMsg);
	interactiveTestRegisterRequest(&sendMsg);
	APNetworkSendUnconnected(sendMsg.msg, sendMsg.offset);
	AP_FREE_PROTOCOL_MESSAGE(sendMsg);
	/*
	recvfrom(gSocket, buffer, 1024, 0, (struct sockaddr*)&controllerSockAddr, &controllerSockAddrLen) ;
	APNetworkInitControllerAddr(controllerSockAddr);
	*/
	AP_INIT_PROTOCOL_MESSAGE(sendMsg);
	interactiveTestConfigurationRequest(&sendMsg);
	APNetworkSendUnconnected(sendMsg.msg, sendMsg.offset);
	AP_FREE_PROTOCOL_MESSAGE(sendMsg);
	/*
	recvfrom(gSocket, buffer, 1024, 0, (struct sockaddr*)&controllerSockAddr, &controllerSockAddrLen) ;
	APNetworkInitControllerAddr(controllerSockAddr);
	*/
	AP_INIT_PROTOCOL_MESSAGE(sendMsg);
	interactiveTestConfigurationReport(&sendMsg);
	APNetworkSendUnconnected(sendMsg.msg, sendMsg.offset);
	AP_FREE_PROTOCOL_MESSAGE(sendMsg);
}

int main()
{
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
	strcpy(gControllerAddr,"127.0.0.1\0");
	strcpy(gAPName,"TESTAP\0");
	strcpy(gAPBoardData,"TESTBOARDDATA\0");
	strcpy(gAPDescriptor,"DESCRIPTOR\0");
	gAPID = 12345;
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