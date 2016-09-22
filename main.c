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
	APNetworkInit();
	APNetworkInitLocalAddr(&gAPIPAddr, gAPMACAddr, &gAPDefaultGateway);
	// printf("IP:  %u.%u.%u.%u\n", (u8)(gAPIPAddr >> 24), (u8)(gAPIPAddr >> 16),  (u8)(gAPIPAddr >> 8),  (u8)(gAPIPAddr >> 0));
	// printf("MAC:  %02x:%02x:%02x:%02x:%02x:%02x\n", gAPMACAddr[0], gAPMACAddr[1], gAPMACAddr[2], gAPMACAddr[3], gAPMACAddr[4], gAPMACAddr[5]);
	// printf("gateway:  %u.%u.%u.%u\n", (u8)(gAPDefaultGateway >> 24), (u8)(gAPDefaultGateway >> 16),  (u8)(gAPDefaultGateway >> 8),  (u8)(gAPDefaultGateway >> 0));
	
	APProtocolMessage sendMsg;
	AP_INIT_PROTOCOL(sendMsg);
	APAssembleDiscoveryRequest(&sendMsg);
	APNetworkSendToBroad(sendMsg);
	AP_FREE_PROTOCOL_MESSAGE(sendMsg);
}