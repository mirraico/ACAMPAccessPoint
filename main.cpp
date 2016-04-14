#include "AcampProtocol.h"
#include "AcampTransmission.h"

int main()
{
    udp_init("127.0.0.1", 1080);

    //create package
    acamp_element* eles[3];
    int eles_len = 0;
    for(int i = 0; i < 3; i++)
    {
        char ele_data[5];
        ele_data[0] = 'e';ele_data[1] = 'l';ele_data[2] = 'e';
        ele_data[3] = 'm';ele_data[4] = i + 49;
        eles[i] = make_acamp_element(0xffff, 5, (u8*)&ele_data);
        eles_len += (sizeof(eles[i]->type) + sizeof(eles[i]->len) + 5);
    }
    acamp_header* head = make_acamp_header(0x12345678, 0x11,  0x22, 0x3344,
                                              0x55667788, 0x9900, sizeof(acamp_header) + eles_len);
    u8* buf = acamp_encapsulate(head, eles, 3);
//    udp_send(buf, head->msg_len);
//    free(buf);
//    u8* recv_buf = (u8*)malloc(sizeof(u8) * 1024);
//    int len = udp_recv(recv_buf, 1024);
//    acamp_header* recv_header;
//    acamp_element** recv_element;
//    int recv_num;
//    acamp_parse(buf, recv_header, recv_element, &recv_num);
    acamp_header *recv_header;
    acamp_element** recv_element;
    int recv_num;
    acamp_parse(buf, recv_header, recv_element, &recv_num);
    printf("%x ", recv_header->msg_len);
    printf("%x ", recv_header->preamble);
    printf("%x ", recv_header->version);
    printf("%x ", recv_header->type);
    printf("%x ", recv_header->apid);
    printf("%x ", recv_header->seq_num);
    printf("%x ", recv_header->msg_type);
    for (int i = 0; i <3; i ++)
    {
        printf("%x ", recv_element[i]->type);
        printf("%d ", recv_element[i]->len);
        printf("%s ", recv_element[i]->data);
    }
}


