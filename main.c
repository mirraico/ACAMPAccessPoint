#include "common.h"
#include "ap.h"
#include "network.h"

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

int main()
{
	APInit();

	if(!APParseSettingsFile()){
		return 0;
	}
	// printf("gAPName: %s\n", gAPName);
	// printf("gAPDescriptor: %s\n", gAPDescriptor);
	// printf("gDiscoveryType: %d\n", gDiscoveryType);
	// printf("gSSID: %s\n", gSSID);
	// printf("gSuppressSSID: %d\n", gSuppressSSID);
	// printf("gHardwareMode: %d\n", gHardwareMode);
	// printf("gChannel: %d\n", gChannel);
	// printf("gSecuritySetting: %d\n", gSecuritySetting);
	
	if(!APNetworkInit()){
		return 0;
	}
	if(!APNetworkInitLocalAddr(&gAPIPAddr, gAPMACAddr, &gAPDefaultGateway)){
		return 0;
	}
	// printf("IP:  %u.%u.%u.%u\n", (u8)(gAPIPAddr >> 24), (u8)(gAPIPAddr >> 16),  (u8)(gAPIPAddr >> 8),  (u8)(gAPIPAddr >> 0));
	// printf("MAC:  %02x:%02x:%02x:%02x:%02x:%02x\n", gAPMACAddr[0], gAPMACAddr[1], gAPMACAddr[2], gAPMACAddr[3], gAPMACAddr[4], gAPMACAddr[5]);
	// printf("gateway:  %u.%u.%u.%u\n", (u8)(gAPDefaultGateway >> 24), (u8)(gAPDefaultGateway >> 16),  (u8)(gAPDefaultGateway >> 8),  (u8)(gAPDefaultGateway >> 0));


	APStateTransition nextState = AP_ENTER_DISCOVERY;

	/* start acamp state machine */	
	AP_REPEAT_FOREVER {
		switch(nextState) {
			case AP_ENTER_DISCOVERY:
				// nextState = APEnterDiscovery();
				break;
			case AP_ENTER_REGISTER:
				// nextState = APEnterRegister();
				break;
			case AP_ENTER_CONFIGURE:
				// nextState = APEnterConfigure();
				break;
			case AP_ENTER_CONFIGURE_RETRY:
				// nextState = APEnterConfigureRetry();
				break;
			case AP_ENTER_RUN:
				// nextState = APEnterRun();
				break;
			case AP_ENTER_DOWN:
				// nextState = APEnterDown();
				break;
		}
	}

	/*
	APProtocolMessage sendMsg;
	AP_INIT_PROTOCOL(sendMsg);
	APAssembleDiscoveryRequest(&sendMsg);
	APNetworkSendToBroad(sendMsg);
	AP_FREE_PROTOCOL_MESSAGE(sendMsg);
	*/

	return 0;
}