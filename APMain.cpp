#include "Common.h"
//#include "AcampAP.h"
#include "AcampNetwork.h"
//#include "AcampProtocol.h"
#include "AcampRegisterState.h"
extern APStateTransition  APEnterDiscovery();
//extern APStateTransition  APEnterRegister();

void APInitConfiguration();
/********Carl   - 05/ 2016    ********************/
//待讨论： 
//1. AcmpAP.h与AcampProtocol.h的功能重复？？？
//2. log 文件？？
//3.  extern的语句
//4. sprint??
//5. Networking.c 文件 



int main()
{
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
			   nextState = APEnterRegister();
				break;
			case AP_RUN:
				break;
			case AP_DOWN:
				break;
		}
	}

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