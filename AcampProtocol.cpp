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


int acamp_parse(u8* buf,
                   acamp_header* header, acamp_element* element[], int array_len)
{    
    u8* ptr = buf;
    memcpy(header, ptr, sizeof(acamp_header));
    header->preamble = ntohl(header->preamble);
    header->apid = ntohs(header->apid);
    header->seq_num = ntohl(header->seq_num);
    header->msg_type = ntohs(header->msg_type);
    header->msg_len = ntohs(header->msg_len);

    ptr += sizeof(acamp_header);
    int ele_cnt = 0;
    for (int i = 0; i < array_len; i++)
    {
        if(ptr - buf >= header->msg_len) break;
        element[i] = (acamp_element*)malloc(sizeof(acamp_element));
        u16 net_ele_type;
        memcpy(&net_ele_type, ptr, sizeof(u16));
        element[i]->type = ntohs(net_ele_type);
        ptr += sizeof(u16);
        u16 net_ele_len;
        memcpy(&net_ele_len, ptr, sizeof(u16));
        element[i]->len = ntohs(net_ele_len);
        ptr += sizeof(u16);
        element[i]->data = (u8*)malloc(element[i]->len * sizeof(u8));
        memcpy(element[i]->data, ptr, element[i]->len * sizeof(u8));
        ptr += element[i]->len * sizeof(u8);
        ele_cnt++;
    }
    return ele_cnt;
}
