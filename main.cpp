#include "Common.h"
#include "AcampProtocol.h"
#include "AcampTransmission.h"

int main(int argc,char *argv[])
{
    //udp init
    if(argc == 3)
        udp_init(argv[1], atoi(argv[2]));
    else
        udp_init("127.0.0.1", 1080);

    gAPName = "TESTAP\0";
    gAPID = 12345;

    APProtocolMessage *msgElems;
	int msgElemCount = 1;
    AP_CREATE_PROTOCOL_ARRAY(msgElems,  msgElemCount);
    APAssembleMsgElemAPName(&(msgElems[0]));

    APProtocolMessage sendMsg; sendMsg.msg = NULL;
    APAssembleMessage(&sendMsg, 12345, MSGTYPE_CONFIGURATION_REQUEST, msgElems, msgElemCount);

    udp_send(sendMsg.msg, sendMsg.offset);

    AP_FREE_PROTOCOL_MESSAGE(sendMsg);
}
