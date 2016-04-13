#include "AcampProtocol.h"

acamp_header* make_acamp_header(u32 preamble, u8 version,
                                u8 type, u16 apid, u32 seq_num,
                                u16 msg_type, u16 msg_len)
{
    acamp_header* header = (acamp_header*)malloc(sizeof(acamp_header));
    header->preamble = preamble;
    header->version = version;
    header->type = type;
    header->apid = apid;
    header->seq_num = seq_num;
    header->msg_type = msg_type;
    header->msg_len = msg_len;
    return header;
}

acamp_element* make_acamp_element(u16 type, u16 len, u8* data)
{
    acamp_element* element = (acamp_element*)malloc(sizeof(acamp_element));
    element->type = type;
    element->len = len;
    element->data = (u8*)malloc(sizeof(u8) * len);
    memcpy(element->data, data, len);
    return element;
}

u8* acamp_encapsulate(acamp_header* header, acamp_element* element[], int ele_num)
{
    u8* buf= (u8*)malloc(header->msg_len * sizeof(u8));
    u8* ptr = buf;
    u32 net_preamble = htonl(header->preamble);
    memcpy(ptr, &net_preamble, sizeof(net_preamble));
    ptr += sizeof(net_preamble);
    memcpy(ptr, &header->version, sizeof(header->version));
    ptr += sizeof(header->version);
    memcpy(ptr, &header->type, sizeof(header->type));
    ptr += sizeof(header->type);
    u16 net_apid = htons(header->apid);
    memcpy(ptr, &net_apid, sizeof(net_apid));
    ptr += sizeof(net_apid);
    u32 net_seq = htonl(header->seq_num);
    memcpy(ptr, &net_seq, sizeof(net_seq));
    ptr += sizeof(net_seq);
    u16 net_msg_type = htons(header->msg_type);
    memcpy(ptr, &net_msg_type, sizeof(net_msg_type));
    ptr += sizeof(net_msg_type);
    u16 net_msg_len = htons(header->msg_len);
    memcpy(ptr, &net_msg_len, sizeof(net_msg_len));
    ptr += sizeof(net_msg_len);

    for (int i = 0; i < ele_num; i++)
    {
        u16 net_ele_type = htons(element[i]->type);
        memcpy(ptr, &net_ele_type, sizeof(net_ele_type));
        ptr += sizeof(net_ele_type);
        u16 net_ele_len = htons(element[i]->len);
        memcpy(ptr, &net_ele_len, sizeof(net_ele_len));
        ptr += sizeof(net_ele_len);
        memcpy(ptr, (u8*)element[i]->data, sizeof(u8)*element[i]->len);
        ptr += sizeof(u8)*element[i]->len;
    }
    return buf;
}


APBool acamp_parse(u8* buf,
                   acamp_header* header, acamp_element* element[], int* ele_num)
{
    u8* ptr = buf;
    header = (acamp_header*)malloc(sizeof(acamp_header));
    memcpy(header, ptr, sizeof(acamp_header));
    ptr += sizeof(acamp_header);
    int num = 0;
    while (ptr - buf < header->msg_len)
    {
        element[num] = (acamp_element*)malloc(sizeof(acamp_element));
        memcpy(&element[num]->type, ptr, sizeof(u16));
        ptr += sizeof(u16);
        memcpy(&element[num]->len, ptr, sizeof(u16));
        ptr += sizeof(u16);
        element[num]->data = (u8*)malloc(element[num]->len * sizeof(u8));
        memcpy(element[num]->data, ptr, element[num]->len * sizeof(u8));
        num++;
    }
    ele_num = &num;
    return AP_TRUE;
}
