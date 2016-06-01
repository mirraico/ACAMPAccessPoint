#include "Common.h"
#include "AcampAP.h"
#include "AcampNetwork.h"
#include "AcampProtocol.h"
#include <pthread.h>

void APInitConfiguration();

pthread_t tid;

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
	
	bool foundAC = false;
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
				foundAC = true;
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
	if(!foundAC || gControllerIPAddr == 0) return AP_FALSE;
	gNetworkControllerAddr = gControllerIPAddr;
	APNetworkInitControllerAddr();
	return AP_TRUE;
}

APBool APReceiveDiscoveryResponse()
{
	u8 buffer[AP_BUFFER_SIZE];
	int readBytes; APNetworkAddress controllerAddr;
	if(!APNetworkReceiveFromBroadUnconnected(buffer, AP_BUFFER_SIZE - 1, &readBytes, &controllerAddr)) {
		return AP_FALSE;
	}
	u32 seqNum;
	if(!APParseDiscoveryResponseMessage(buffer, readBytes, &seqNum)) {
		return AP_FALSE;
	}
	return AP_TRUE;
}

APBool APParseRegisterResponseMessage(u8 *msg,
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
	if(header.msgType != MSGTYPE_REGISTER_RESPONSE)
		return AP_FALSE;
	*seqNumPtr = header.seqNum;

	bool getAPID = false;
	while((msgLen - completeMsg.offset) > 0) {
		u16 elemType = 0;
		u16 elemLen = 0;
		if(!APParseFormatMsgElement(&completeMsg, &elemType, &elemLen)) {
			return AP_FALSE;
		}
		switch(elemType) {
			case MSGELETYPE_RESULT_CODE:
				int res;
				if(!APParseMsgElemResultCode(&completeMsg, elemLen, &res) || res)
					return AP_FALSE;
				break;
			case MSGELETYPE_ASSIGNED_APID:
				if(!APParseMsgElemAssignedAPID(&completeMsg, elemLen))
					return AP_FALSE;
				getAPID = true;
				break;
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
				getAPID = true;
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
	if(!getAPID) return AP_FALSE;
	return AP_TRUE;
}

APBool APReceiveRegisterResponse()
{
	u8 buffer[AP_BUFFER_SIZE];
	int readBytes; APNetworkAddress controllerAddr;
	if(!APNetworkReceiveUnconnected(buffer, AP_BUFFER_SIZE - 1, &readBytes, &controllerAddr)) {
		return AP_FALSE;
	}
	u32 seqNum;
	if(!APParseRegisterResponseMessage(buffer, readBytes, &seqNum)) {
		return AP_FALSE;
	}
	return AP_TRUE;
}

APBool APParseConfigurationDeliverMessage(u8 *msg,
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
	if(header.msgType != MSGTYPE_CONFIGURATION_DELIVER)
		return AP_FALSE;
	*seqNumPtr = header.seqNum;

	while((msgLen - completeMsg.offset) > 0) {
		u16 elemType = 0;
		u16 elemLen = 0;
		if(!APParseFormatMsgElement(&completeMsg, &elemType, &elemLen)) {
			return AP_FALSE;
		}
		switch(elemType) {
			case MSGELETYPE_SSID:
				if(!APParseMsgElemSSID(&completeMsg, elemLen))
					return AP_FALSE;
				break;
			case MSGELETYPE_CHANNEL:
				if(!APParseMsgElemChannel(&completeMsg, elemLen))
					return AP_FALSE;
				break;
			case MSGELETYPE_HW_MODE:
				if(!APParseMsgElemHardwareMode(&completeMsg, elemLen))
					return AP_FALSE;
				break;
			case MSGELETYPE_SUPPRESS_SSID:
				if(!APParseMsgElemSuppressSSID(&completeMsg, elemLen))
					return AP_FALSE;
				break;
			case MSGELETYPE_SECURITY_SETTING:
				if(!APParseMsgElemSecuritySetting(&completeMsg, elemLen))
					return AP_FALSE;
				break;
			case MSGELETYPE_WPA_VERSION:
				if(!APParseMsgElemWPAVersion(&completeMsg, elemLen))
					return AP_FALSE;
				break;
			case MSGELETYPE_WPA_PASSPHRASE:
				if(!APParseMsgElemWPAPassphrase(&completeMsg, elemLen))
					return AP_FALSE;
				break;
			case MSGELETYPE_WPA_KEY_MANAGEMENT:
				if(!APParseMsgElemWPAKeyManagement(&completeMsg, elemLen))
					return AP_FALSE;
				break;
			case MSGELETYPE_WPA_PAIRWISE:
				if(!APParseMsgElemWPAPairwise(&completeMsg, elemLen))
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

APBool APReceiveConfigurationDeliver()
{
	u8 buffer[AP_BUFFER_SIZE];
	int readBytes; APNetworkAddress controllerAddr;
	if(!APNetworkReceiveUnconnected(buffer, AP_BUFFER_SIZE - 1, &readBytes, &controllerAddr)) {
		return AP_FALSE;
	}
	u32 seqNum;
	if(!APParseConfigurationDeliverMessage(buffer, readBytes, &seqNum)) {
		return AP_FALSE;
	}
	return AP_TRUE;
}


APBool APAssembleDiscoveryRequest(APProtocolMessage *messagesPtr)
{
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0;
	AP_CREATE_PROTOCOL_ARRAY_AND_INIT(msgElems,  msgElemCount);
	
	return APAssembleMessage (messagesPtr, 
				 15432,
				 MSGTYPE_DISCOVER_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

APBool APAssembleRegisterRequest(APProtocolMessage *messagesPtr)
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

APBool APAssembleConfigurationRequest(APProtocolMessage *messagesPtr)
{
	if(messagesPtr == NULL) return AP_FALSE;

	APProtocolMessage *msgElems;
	int msgElemCount = 9, k = 0;
	AP_CREATE_PROTOCOL_ARRAY_AND_INIT(msgElems,  msgElemCount);

	if(
		(!(APAssembleMsgElemSSID(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemChannel(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemHardwareMode(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemSuppressSSID(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemSecuritySetting(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemWPAVersion(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemWPAPassphrase(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemWPAKeyManagement(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemWPAPairwise(&(msgElems[k++]))))
		)
	{
		int i;
		for(i = 0; i < k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		AP_FREE_OBJECT(msgElems);
		return AP_FALSE;
	}

	return APAssembleMessage (messagesPtr,
				 15434,
				 MSGTYPE_CONFIGURATION_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

APBool APAssembleConfigurationReport(APProtocolMessage *messagesPtr)
{
	if(messagesPtr == NULL) return AP_FALSE;

	APProtocolMessage *msgElems;
	int msgElemCount = 9, k = 0;
	AP_CREATE_PROTOCOL_ARRAY_AND_INIT(msgElems,  msgElemCount);

	if(
		(!(APAssembleMsgElemSSID(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemChannel(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemHardwareMode(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemSuppressSSID(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemSecuritySetting(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemWPAVersion(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemWPAPassphrase(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemWPAKeyManagement(&(msgElems[k++])))) ||
		(!(APAssembleMsgElemWPAPairwise(&(msgElems[k++]))))
		)
	{
		int i;
		for(i = 0; i < k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		AP_FREE_OBJECT(msgElems);
		return AP_FALSE;
	}

	return APAssembleMessage (messagesPtr,
				 15434,
				 MSGTYPE_CONFIGURATION_REPORT,
				 msgElems,
				 msgElemCount
	);
}

APBool APAssembleKeepAliveRequest(APProtocolMessage *messagesPtr)
{
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0;
	AP_CREATE_PROTOCOL_ARRAY_AND_INIT(msgElems,  msgElemCount);
	
	return APAssembleMessage (messagesPtr, 
				 15432,
				 MSGTYPE_KEEP_ALIVE_REQUEST,
				 msgElems,
				 msgElemCount
	);
}

APBool APAssembleStatisticReport(APProtocolMessage *messagesPtr)
{
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0;
	AP_CREATE_PROTOCOL_ARRAY_AND_INIT(msgElems,  msgElemCount);
	
	return APAssembleMessage(messagesPtr, 
				 15432,
				 MSGTYPE_STATISTIC_REPORT,
				 msgElems,
				 msgElemCount
	);
}

APBool APAssembleUnregisterResponse(APProtocolMessage *messagesPtr)
{
	if(messagesPtr == NULL) return AP_FALSE;
	
	APProtocolMessage *msgElems;
	int msgElemCount = 0;
	AP_CREATE_PROTOCOL_ARRAY_AND_INIT(msgElems,  msgElemCount);
	
	return APAssembleMessage(messagesPtr, 
				 15432,
				 MSGTYPE_UNREGISTER_RESPONSE,
				 msgElems,
				 msgElemCount
	);
}

void *echo_func(void *arg) 
{
	APProtocolMessage sendMsg;
	AP_REPEAT_FOREVER {
		sleep(15);
		AP_INIT_PROTOCOL_MESSAGE(sendMsg);
		APAssembleStatisticReport(&sendMsg);
		APNetworkSendUnconnected(sendMsg);
		AP_FREE_PROTOCOL_MESSAGE(sendMsg);
		sleep(15);
		AP_INIT_PROTOCOL_MESSAGE(sendMsg);
		APAssembleKeepAliveRequest(&sendMsg);
		APNetworkSendUnconnected(sendMsg);
		AP_FREE_PROTOCOL_MESSAGE(sendMsg);
	}
	pthread_exit(NULL); 
}

void interactiveTest()
{
	//init
	APNetworkInit();
	APNetworkInitLocalAddr();
	APInitConfiguration();

	//discover
	APProtocolMessage sendMsg;
	AP_INIT_PROTOCOL_MESSAGE(sendMsg);
	APAssembleDiscoveryRequest(&sendMsg);
	APNetworkSendToBroadUnconnected(sendMsg);
	AP_FREE_PROTOCOL_MESSAGE(sendMsg);

	APReceiveDiscoveryResponse();

	//register
	AP_INIT_PROTOCOL_MESSAGE(sendMsg);
	APAssembleRegisterRequest(&sendMsg);
	APNetworkSendUnconnected(sendMsg);
	AP_FREE_PROTOCOL_MESSAGE(sendMsg);

	APReceiveRegisterResponse();

	//configurate
	AP_INIT_PROTOCOL_MESSAGE(sendMsg);
	APAssembleConfigurationRequest(&sendMsg);
	APNetworkSendUnconnected(sendMsg);
	AP_FREE_PROTOCOL_MESSAGE(sendMsg);

	APReceiveConfigurationDeliver();

	AP_INIT_PROTOCOL_MESSAGE(sendMsg);
	APAssembleConfigurationReport(&sendMsg);
	APNetworkSendUnconnected(sendMsg);
	AP_FREE_PROTOCOL_MESSAGE(sendMsg);
	
	//echo pthread
	if (pthread_create(&tid, NULL, echo_func, NULL) != 0) {
		 printf("Create thread error!\n");
		 exit(1);
	 }
	
	AP_REPEAT_FOREVER {
		u8 buffer[AP_BUFFER_SIZE];
		int readBytes; APNetworkAddress controllerAddr;
		APNetworkReceiveUnconnected(buffer, AP_BUFFER_SIZE - 1, &readBytes, &controllerAddr);

		APHeaderVal header;
		APProtocolMessage completeMsg;
		completeMsg.msg = (u8*)buffer;
		completeMsg.offset = 0;

		APParseControlHeader(&completeMsg, &header);
		switch(header.msgType) {
			case MSGTYPE_CONFIGURATION_DELIVER:
				AP_INIT_PROTOCOL_MESSAGE(sendMsg);
				APAssembleConfigurationReport(&sendMsg);
				APNetworkSendUnconnected(sendMsg);
				AP_FREE_PROTOCOL_MESSAGE(sendMsg);
				break;
			case MSGTYPE_CONFIGURATION_REQUEST:
				AP_INIT_PROTOCOL_MESSAGE(sendMsg);
				APAssembleConfigurationReport(&sendMsg);
				APNetworkSendUnconnected(sendMsg);
				AP_FREE_PROTOCOL_MESSAGE(sendMsg);
				break;
			case MSGTYPE_STATISTIC_REQUEST:
				AP_INIT_PROTOCOL_MESSAGE(sendMsg);
				APAssembleStatisticReport(&sendMsg);
				APNetworkSendUnconnected(sendMsg);
				AP_FREE_PROTOCOL_MESSAGE(sendMsg);
				break;
			case MSGTYPE_UNREGISTER_REQUEST:
				AP_INIT_PROTOCOL_MESSAGE(sendMsg);
				APAssembleUnregisterResponse(&sendMsg);
				APNetworkSendUnconnected(sendMsg);
				AP_FREE_PROTOCOL_MESSAGE(sendMsg);
				break;
			default:
				break;
		}
	}
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
	
	AP_CREATE_STRING_SIZE(gSSID, 11);
	strcpy(gSSID, "xiangzi123\0");
	gChannel = 7;
	gHwMode = 2 ;
	gSuppressSSID = 0;
	gSecuritySetting = 4;
	gWPAVersion = 2;	
	AP_CREATE_STRING_SIZE(gWPAPasswd, 9);
	strcpy(gWPAPasswd, "12345678\0");
	gWPAKeyManagement = 15;
	gWPAPairwise = 1;
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
