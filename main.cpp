#include "AcampProtocol.h"
#include "AcampTransmission.h"

void all_msg_element_test()
{
    /***all msg element test***/

    //create package
    int package_len = 0;
    //create element
    acamp_element* eles[23];
    u8 result_code = RESULTCODE_FAILURE;
    eles[0] = make_acamp_element(MSGELETYPE_RESULT_CODE, sizeof(u8), (u8*)&result_code);
    package_len += (ELEMENT_HEADER_LEN + sizeof(u8));

    u32 reason_code = REASONCODE_AP_NO_RESPONE;
    reason_code = htonl(reason_code);
    eles[1] = make_acamp_element(MSGELETYPE_REASON_CODE, sizeof(u32), (u8*)&reason_code);
    package_len += (ELEMENT_HEADER_LEN + sizeof(u32));

    u8 apid = 111;
    eles[2] = make_acamp_element(MSGELETYPE_ASSIGNED_APID, sizeof(u8), (u8*)&apid);
    package_len += (ELEMENT_HEADER_LEN + sizeof(u8));

    u8 ap_mac_addr[6] = {1, 2, 3, 4, 5, 6};
    eles[3] = make_acamp_element(MSGELETYPE_AP_MAC_ADDR, sizeof(u8)*6, (u8*)&ap_mac_addr);
    package_len += (ELEMENT_HEADER_LEN + sizeof(u8)*6);

    u8 ap_inet_addr[4] = {192, 168, 56, 2};
    eles[4] = make_acamp_element(MSGELETYPE_AP_INET_ADDR, sizeof(u8)*4, (u8*)&ap_inet_addr);
    package_len += (ELEMENT_HEADER_LEN + sizeof(u8)*4);

    char ap_name[] = "115AP\0";
    eles[5] = make_acamp_element(MSGELETYPE_AP_NAME, strlen(ap_name), (u8*)&ap_name);
    package_len += (ELEMENT_HEADER_LEN + strlen(ap_name));

    char ap_description[] = "a 115 handsome AP\0";
    eles[6] = make_acamp_element(MSGELETYPE_AP_DESCRIPTION, strlen(ap_description), (u8*)&ap_description);
    package_len += (ELEMENT_HEADER_LEN + strlen(ap_description));

    char ap_location[] = "on your right side\0";
    eles[7] = make_acamp_element(MSGELETYPE_AP_LOCATION, strlen(ap_location), (u8*)&ap_location);
    package_len += (ELEMENT_HEADER_LEN + strlen(ap_location));

    char ap_board[] = "made in china\0";
    eles[8] = make_acamp_element(MSGELETYPE_AP_BOARD_DATA, strlen(ap_board), (u8*)&ap_board);
    package_len += (ELEMENT_HEADER_LEN + strlen(ap_board));

    u8 ac_mac_addr[6] = {6, 5, 4, 3, 2, 1};
    eles[9] = make_acamp_element(MSGELETYPE_AP_MAC_ADDR, sizeof(u8)*6, (u8*)&ac_mac_addr);
    package_len += (ELEMENT_HEADER_LEN + sizeof(u8)*6);

    u8 ac_inet_addr[4] = {192, 168, 56, 102};
    eles[10] = make_acamp_element(MSGELETYPE_AP_INET_ADDR, sizeof(u8)*4, (u8*)&ac_inet_addr);
    package_len += (ELEMENT_HEADER_LEN + sizeof(u8)*4);

    u32 time_stamp = 0x12345678;
    eles[11] = make_acamp_element(MSGELETYPE_TIME_STAMP, sizeof(u8)*4, (u8*)&time_stamp);
    package_len += (ELEMENT_HEADER_LEN + sizeof(u8)*4);

    u8 radio_id = 101;
    //wlan information
    u8 wlan_information[200]; int wlan_information_len = 0;
    {
        u8 wlan_id = 102;
        u16 capability = 0x5555;
        capability = htons(capability);
        u8 key_index = 1; u8 key_status = 2;
        u16 key_length = 8; u8 key[] = "ouycouyc\0";
        key_length = htons(key_length);
        u8 group_tsc[6] = {0, 0, 0, 0, 0, 0};
        u8 qos = WLANINFO_QOS_BESTEFFORT; u8 auth_type = WLANINFO_AUTHTYPE_WEP;
        u8 suppress_ssid = WLANINFO_SUPRESS_SSID_SUPRESS;
        char ssid[] = "115123123\0";

        u8* ptr = wlan_information;
        memcpy(ptr, &radio_id, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &wlan_id, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &capability, sizeof(u16)); ptr += sizeof(u16);
        memcpy(ptr, &key_index, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &key_status, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &key_length, sizeof(u16)); ptr += sizeof(u16);
        memcpy(ptr, key, sizeof(u8)*key_length); ptr += sizeof(u8)*key_length;
        memcpy(ptr, group_tsc, sizeof(u8)*6); ptr += sizeof(u8)*6;
        memcpy(ptr, &qos, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &auth_type, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &suppress_ssid, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, ssid, strlen(ssid)); ptr += strlen(ssid);
        wlan_information_len = ptr - wlan_information;
    }
    eles[12] = make_acamp_element(MSGELETYPE_WLAN_INFO, wlan_information_len, (u8*)&wlan_information);
    package_len += (ELEMENT_HEADER_LEN + wlan_information_len);

    //ap radio information
    u8 radio_information[200]; int radio_information_len = 0;
    {
        u8 short_preamble = RADIOINFO_SHORTPREAMBLE_SUPPORTED;
        u8 bssid_num = 5; u8 dtim_period = 8;
        u8 bssid[6] = {6, 6, 6, 6, 6, 6};
        u16 beacon_period = 30; u32 country = 86;
        beacon_period = htons(beacon_period);
        country = htonl(country);

        u8* ptr = radio_information;
        memcpy(ptr, &radio_id, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &short_preamble, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &bssid_num, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &dtim_period, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, bssid, sizeof(u8)*6); ptr += sizeof(u8)*6;
        memcpy(ptr, &beacon_period, sizeof(u16)); ptr += sizeof(u16);
        memcpy(ptr, &country, sizeof(u32)); ptr += sizeof(u32);
        radio_information_len = ptr - radio_information;
    }
    eles[13] = make_acamp_element(MSGELETYPE_AP_RADIO_INFO, radio_information_len, (u8*)&radio_information);
    package_len += (ELEMENT_HEADER_LEN + radio_information_len);

    //antenna
    u8 antenna[200]; int antenna_len = 0;
    {
        u8 diversity = ANTENNA_DIVERSITY_ENABLED;
        u8 combiner = ANTENNA_COMBINER_RIGHTSECTORIEZD; u8 antenna_cnt = 3;
        u32 selection = ANTENNA_SELECTION_INTER;
        selection = htonl(selection);

        u8* ptr = antenna;
        memcpy(ptr, &radio_id, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &diversity, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &combiner, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &antenna_cnt, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &selection, sizeof(u32)); ptr += sizeof(u32);
        antenna_len = ptr - antenna;
    }
    eles[14] = make_acamp_element(MSGELETYPE_ANTENNA, antenna_len, (u8*)&antenna);
    package_len += (ELEMENT_HEADER_LEN + antenna_len);

    //tx power
    u8 tx_power[200]; int tx_power_len = 0;
    {
        u8 num_level = 3;
        u16 level[3] = {100, 200, 300};
        level[0] = htons(level[0]);
        level[1] = htons(level[1]);
        level[2] = htons(level[2]);

        u8* ptr = tx_power;
        memcpy(ptr, &radio_id, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &num_level, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, level, sizeof(u16)*3); ptr += sizeof(u16)*3;
        tx_power_len = ptr - tx_power;
    }
    eles[15] = make_acamp_element(MSGELETYPE_TX_POWER, tx_power_len, (u8*)&tx_power);
    package_len += (ELEMENT_HEADER_LEN + tx_power_len);

    //multi-domain capability
    u8 multi_domain_cap[200]; int multi_domain_cap_len = 0;
    {
        u8 reserved = 0;
        u16 first_channel = 0; u16 channel_num = 13; u16 max_tx_power_lev = 1368;
        first_channel = htons(first_channel);
        channel_num = htons(channel_num);
        max_tx_power_lev = htons(max_tx_power_lev);

        u8* ptr = multi_domain_cap;
        memcpy(ptr, &radio_id, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &reserved, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &first_channel, sizeof(u16)); ptr += sizeof(u16);
        memcpy(ptr, &channel_num, sizeof(u16)); ptr += sizeof(u16);
        memcpy(ptr, &max_tx_power_lev, sizeof(u16)); ptr += sizeof(u16);
        multi_domain_cap_len = ptr - multi_domain_cap;
    }
    eles[16] = make_acamp_element(MSGELETYPE_MULTI_DOMAIN_CAP, multi_domain_cap_len, (u8*)&multi_domain_cap);
    package_len += (ELEMENT_HEADER_LEN + multi_domain_cap_len);

    //supported rates
    u8 supported_rates[200]; int supported_rates_len = 0;
    {
        u16 rates = 9600;
        rates = htons(rates);

        u8* ptr = supported_rates;
        memcpy(ptr, &radio_id, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, &rates, sizeof(u16)); ptr += sizeof(u16);
        supported_rates_len = ptr - supported_rates;
    }
    eles[17] = make_acamp_element(MSGELETYPE_SUPPORTED_RATES, supported_rates_len, (u8*)&supported_rates);
    package_len += (ELEMENT_HEADER_LEN + supported_rates_len);

    //add mac acl entry
    u8 add_mac_acl_entry[200]; int add_mac_acl_entry_len = 0;
    {
        u8 num = 2;
        u8 mac[12] = {1, 2, 3, 4, 5, 6, 6, 5, 4, 3, 2, 1};

        u8* ptr = add_mac_acl_entry;
        memcpy(ptr, &num, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, mac, sizeof(u8)*12); ptr += sizeof(u8)*12;
        add_mac_acl_entry_len = ptr - add_mac_acl_entry;
    }
    eles[18] = make_acamp_element(MSGELETYPE_ADD_MAC_ACL_ENTRY, add_mac_acl_entry_len, (u8*)&add_mac_acl_entry);
    package_len += (ELEMENT_HEADER_LEN + add_mac_acl_entry_len);

    //del mac acl entry
    u8 del_mac_acl_entry[200]; int del_mac_acl_entry_len = 0;
    {
        u8 num = 2;
        u8 mac[12] = {1, 2, 3, 4, 5, 6, 6, 5, 4, 3, 2, 1};

        u8* ptr = del_mac_acl_entry;
        memcpy(ptr, &num, sizeof(u8)); ptr += sizeof(u8);
        memcpy(ptr, mac, sizeof(u8)*12); ptr += sizeof(u8)*12;
        del_mac_acl_entry_len = ptr - del_mac_acl_entry;
    }
    eles[19] = make_acamp_element(MSGELETYPE_DEL_MAC_ACL_ENTRY, del_mac_acl_entry_len, (u8*)&del_mac_acl_entry);
    package_len += (ELEMENT_HEADER_LEN + del_mac_acl_entry_len);

    u8 stat_mac[6] = {1, 2, 3, 4, 5, 6};
    eles[20] = make_acamp_element(MSGELETYPE_ADD_STATION, sizeof(u8)*6, (u8*)&stat_mac);
    package_len += (ELEMENT_HEADER_LEN + sizeof(u8)*6);

    eles[21] = make_acamp_element(MSGELETYPE_DEL_STATION, sizeof(u8)*6, (u8*)&stat_mac);
    package_len += (ELEMENT_HEADER_LEN + sizeof(u8)*6);

    eles[22] = make_acamp_element(MSGELETYPE_STATION_EVENT, sizeof(u8)*6, (u8*)&stat_mac);
    package_len += (ELEMENT_HEADER_LEN + sizeof(u8)*6);



    //create header
    package_len += HEADER_LEN;
    acamp_header* head = make_acamp_header(PREAMBLE, VERSION, TYPE_CONTROL, 12345,
                                              123456789, MSGTYPE_REGISTER_REQUEST, package_len);
    //send
    u8* send_buf = acamp_encapsulate(head, eles, 23);
    udp_send(send_buf, package_len);

    printf("all test\n");
}

int main(int argc,char *argv[])
{
    /***test***/
    //udp init
    if(argc == 3)
        udp_init(argv[1], atoi(argv[2]));
    else
        udp_init("127.0.0.1", 1080);

    all_msg_element_test();

    /*
    //create package
    int package_len = 0;
    //create element
    acamp_element* eles[2];
    char ap_name[] = "115AP\0";
    eles[0] = make_acamp_element(MSGELETYPE_AP_NAME, strlen(ap_name), (u8*)&ap_name);
    package_len += (ELEMENT_HEADER_LEN + strlen(ap_name));
    char ap_description[] = "position: on your right side\0";
    eles[1] = make_acamp_element(MSGELETYPE_AP_DESCRIPTION, strlen(ap_description), (u8*)&ap_description);
    package_len += (ELEMENT_HEADER_LEN + strlen(ap_description));
    //create header
    package_len += HEADER_LEN;
    acamp_header* head = make_acamp_header(PREAMBLE, VERSION, TYPE_CONTROL, 12345,
                                              123456789, MSGTYPE_REGISTER_REQUEST, package_len);
    //send
    u8* send_buf = acamp_encapsulate(head, eles, 2);
    udp_send(send_buf, package_len);

    //recv
    u8* recv_buf = (u8*)malloc(sizeof(u8) * 1024);
    udp_recv(recv_buf, 1024);
    //memcpy(recv_buf, send_buf, package_len);

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

    */

}


