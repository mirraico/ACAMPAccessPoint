#include "AcampProtocol.h"
#include "AcampTransmission.h"

int main(int argc,char *argv[])
{
    if(argc == 2)
        udp_init(argv[0], atoi(argv[1]));
    else
        udp_init("127.0.0.1", 1080);

    //create package
    int package_len = 0;
    package_len += sizeof(acamp_header);
    acamp_element* eles[3];
    for(int i = 0; i < 3; i++)
    {
        char ele_data[5];
        ele_data[0] = 'e';ele_data[1] = 'l';ele_data[2] = 'e';
        ele_data[3] = 'm';ele_data[4] = i + 49;
        eles[i] = make_acamp_element(0xffff, 5, (u8*)&ele_data);
        package_len += (sizeof(eles[i]->type) + sizeof(eles[i]->len) + 5);
    }
    acamp_header* head = make_acamp_header(0x12345678, 0x11,  0x22, 0x3344,
                                              0x55667788, 0x9900, package_len);
    u8* buf = acamp_encapsulate(head, eles, 3);
    u8* recv_buf = (u8*)malloc(sizeof(u8) * 1024);
//    udp_send(buf, package_len);
//    //free(buf);
//    udp_recv(recv_buf, 1024);
    memcpy(recv_buf, buf, package_len);
    acamp_header recv_header;
    acamp_element* recv_element[10];
    int recv_num = acamp_parse(recv_buf, &recv_header, recv_element, 10);
    printf("preamble: %x\n", recv_header.preamble);
    printf("version: %x\n", recv_header.version);
    printf("type: %x\n", recv_header.type);
    printf("apid: %x\n", recv_header.apid);
    printf("seq_num: %x\n", recv_header.seq_num);
    printf("msg_type: %x\n", recv_header.msg_type);
    printf("msg_len: %d\n", recv_header.msg_len);
    for (int i = 0; i < recv_num; i ++)
    {
        printf("ele_type: %x\n", recv_element[i]->type);
        printf("ele_len: %d\n", recv_element[i]->len);
        char *data = (char*)malloc((recv_element[i]->len+1)*sizeof(char));
        memcpy(data, recv_element[i]->data, recv_element[i]->len);
        data[recv_element[i]->len] = '\0'; //end of string
        printf("ele_data: %s\n", data);
    }
}


