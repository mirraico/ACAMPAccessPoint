#include "AcampProtocol.h"
#include "AcampTransmission.h"

int main()
{
    int fd = udp_init("127.0.0.1", 1080);

    //create package
    acamp_element* eles[3];
    int eles_len = 0;
    for(int i = 0; i < 3; i++)
    {
        char data[8];
        data[0] = 'e';data[1] = 'l';data[2] = 'e';data[3] = 'm';data[4] = (char)i;data[5] = 0;
        eles[i] = make_acamp_element(0xffff, 5, (u8*)&data);
        eles_len += (sizeof(eles[i]->type) + sizeof(eles[i]->len)
                                              + 5);
    }
    acamp_header* head = make_acamp_header(0x12345678, 0x11,  0x22, 0x3344,
                                              0x55667788, 0x9900, sizeof(acamp_header) + eles_len);

    u8* buf = acamp_encapsulate(head, eles, 3);
    udp_send(fd, buf, head->msg_len);
}


