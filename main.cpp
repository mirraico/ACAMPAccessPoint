#include "AcampProtocol.h"
#include "AcampTransmission.h"

int main(int argc,char *argv[])
{
    /***test***/
    //udp init
    if(argc == 3)
        udp_init(argv[1], atoi(argv[2]));
    else
        udp_init("127.0.0.1", 1080);

    //create package
    int package_len = 0;
    //create element
    acamp_element* eles[2];
    char ap_name[] = "115AP\0";
    eles[0] = make_acamp_element(MSGELETYPE_AP_NAME, strlen(ap_name), (u8*)&ap_name);
    package_len += (ELEMENT_LEN + strlen(ap_name));
    char ap_description[] = "position: on your right side\0";
    eles[1] = make_acamp_element(MSGELETYPE_AP_DESCRIPTION, strlen(ap_description), (u8*)&ap_description);
    package_len += (ELEMENT_LEN + strlen(ap_description));
    //create header
    package_len += HEADER_LEN;
    acamp_header* head = make_acamp_header(PREAMBLE, VERSION, TYPE_CONTROL, 12345,
                                              123456789, MSGTYPE_REGISTER_REQUEST, package_len);
    //send
    u8* send_buf = acamp_encapsulate(head, eles, 2);
    udp_send(send_buf, package_len);

    //recv
    u8* recv_buf = (u8*)malloc(sizeof(u8) * 1024);
    //udp_recv(recv_buf, 1024);
    memcpy(recv_buf, send_buf, package_len);

    //parse
    acamp_header recv_header;
    acamp_element* recv_element[10];
    int recv_num = acamp_parse(recv_buf, &recv_header, recv_element, 10);
    printf("msg preamble: 0x%x\n", recv_header.preamble);
    printf("msg version: 0x%x\n", recv_header.version);
    printf("msg type: 0x%x\n", recv_header.type);
    printf("msg apid: %d\n", recv_header.apid);
    printf("msg seq_num: %d\n", recv_header.seq_num);
    printf("msg msg_type: 0x%x\n", recv_header.msg_type);
    printf("msg msg_len: %d\n", recv_header.msg_len);
    for (int i = 0; i < recv_num; i ++)
    {
        printf("element %d type: 0x%x\n", i + 1, recv_element[i]->type);
        printf("element %d len: %d\n", i + 1, recv_element[i]->len);
        //print string
        char *data = (char*)malloc((recv_element[i]->len+1)*sizeof(char));
        memcpy(data, recv_element[i]->data, recv_element[i]->len);
        data[recv_element[i]->len] = '\0'; //end of string
        printf("element %d data: %s\n", i + 1, data);
//        //print hex
//        printf("element %d data: 0x%x\n", i + 1, recv_element[i]->data);
    }
}


