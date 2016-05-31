#include "AcampProtocol.h"
#include "AcampNetwork.h"

int gAPMaxDiscoveries = 10;
int gAPDiscoveryCount;
int gAPDiscoveryInterval = 5;

APBool APAssembleDiscoveryRequest(APProtocolMessage *messagesPtr);
APBool APAssembleMessage(APProtocolMessage *msgPtr,
						 u32 seqNum, u16 msgType, APProtocolMessage msgElems[], const int msgElemNum);

APStateTransition  APEnterDiscovery()
{
	gAPDiscoveryCount = 0;
	APNetworkCloseSocket(gSocket);
	
	//sleep(gAPDiscoveryInterval);
	
	//AP_REPEAT_FOREVER 
	{
		APProtocolMessage sendMsg;
		AP_INIT_PROTOCOL_MESSAGE(sendMsg);
		if(!APAssembleDiscoveryRequest(&sendMsg)) 
		{
			return AP_DISCOVERY;
		}
		
		APNetworkSendToBroadUnconnected(sendMsg.msg, sendMsg.offset);

		AP_FREE_PROTOCOL_MESSAGE(sendMsg);
		
		//sleep(gAPDiscoveryInterval);
	}
	
	return AP_REGISTER;
}

APBool APAssembleDiscoveryRequest(APProtocolMessage *messagesPtr) 
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
		for(i = 0; i <= k; i++) { AP_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		AP_FREE_OBJECT(msgElems);
		return AP_FALSE;
	}
	
	return APAssembleMessage (messagesPtr, 
				 15432,
				 MSGTYPE_DISCOVER_REQUEST,
				 msgElems,
				 msgElemCount
	);
}