#include "Common.h"
#include "AcampAP.h"
#include "AcampNetwork.h"
#include "AcampProtocol.h"

void APInitConfiguration();

int main()
{
	APInitConfiguration();
	APNetworkInit();
	
	APStateTransition nextState = AP_ENTER_REGISTER;

	AP_REPEAT_FOREVER
	{
		switch(nextState)
		{
			case AP_ENTER_REGISTER:
				//nextState = APEnterRegister();
				break;
			case AP_ENTER_RUN:
				//nextState = APEnterRun();
				break;
			case AP_ENTER_DOWN:
				//nextState = APEnterDown();
				break;
		}
	}
	
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

	return 0;
}

void APInitConfiguration()
{
	strcpy(gAddress,"127.0.0.1\0");
	gPort = 1080;
	
	strcpy(gAPName,"TESTAP\0");
	gAPID = 12345;
}
