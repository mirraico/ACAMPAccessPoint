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
    udp_send(buf, head->msg_len);
}


