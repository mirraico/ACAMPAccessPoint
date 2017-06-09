#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "common.h"
#include "error.h"
#include "ap.h"


#define PROTOCOL_PORT 6606

/*
msg
msg len = 16 byte + payload(4 + len)*num byte
-------------------------------------------
0        8        16          24         32
|Version |  Type  |        APID           |
|              Seq Num                    |
|  Msg Type       |           Msg Len     |
|              Reserved                   |
|              *PayLoad                   |
-------------------------------------------
*/

typedef struct {
	u8 version;
	u8 type;
	u16 apid;
	u32 seq_num;
	u16 msg_type;
	u16 msg_len;
} header_val;
#define HEADER_LEN 16
#define CURRENT_VERSION 0x03
#define TYPE_CONTROL 0x00


/*
msg_elem
msg_elem len =  4 + datalen
-------------------------------------------
0        8        16         24          32
|Msg Elements Type|    Msg Elements Len   |
|          *Msg Elements Data             |
-------------------------------------------
*/

typedef struct {
	u16 type;
	u16 len;
	u8* data;
} element_val;
#define ELEMENT_HEADER_LEN 4

/* general type */
typedef struct {
	u8 *msg;
	int offset;
	u16 type;
}protocol_msg;


/* msg type */
#define MSGTYPE_KEEPALIVE_REQUEST 0x0001
#define MSGTYPE_KEEPALIVE_RESPONSE 0x0002
#define MSGTYPE_DISCOVERY_REQUEST 0x0003
#define MSGTYPE_DISCOVERY_RESPONSE 0x0004
#define MSGTYPE_REGISTER_REQUEST 0x0101
#define MSGTYPE_REGISTER_RESPONSE 0x0102
#define MSGTYPE_UNREGISTER_REQUEST 0x0103
#define MSGTYPE_UNREGISTER_RESPONSE 0x0104
#define MSGTYPE_CONFIGURATION_REQUEST 0x0201
#define MSGTYPE_CONFIGURATION_RESPONSE 0x0202
#define MSGTYPE_CONFIGURATION_UPDATE_REQUEST 0x0203
#define MSGTYPE_CONFIGURATION_UPDATE_RESPONSE 0x0204
//#define MSGTYPE_SCAN_REQUEST 0x0301
//#define MSGTYPE_SCAN_RESPONSE 0x0302
//#define MSGTYPE_STATION_REQUEST 0x0303
//#define MSGTYPE_STATION_RESPONSE 0x0304
//#define MSGTYPE_STATISTIC_REQUEST 0x0305
//#define MSGTYPE_STATISTIC_RESPONSE 0x0306
#define MSGTYPE_SYSTEM_REQUEST 0x0307
#define MSGTYPE_SYSTEM_RESPONSE 0x0308

/* msg elem type */
#define MSGELEMTYPE_RESULT_CODE 0x0001
#define MSGELEMTYPE_REASON_CODE 0x0002
#define MSGELEMTYPE_ASSIGNED_APID 0x0003
#define MSGELEMTYPE_DISCOVERY_TYPE 0x0004
#define MSGELEMTYPE_REGISTERED_SERVICE 0x0005
#define MSGELEMTYPE_CONTROLLER_NAME 0x0006
#define MSGELEMTYPE_CONTROLLER_DESCRIPTOR 0x0007
#define MSGELEMTYPE_CONTROLLER_IP_ADDR 0x0008
#define MSGELEMTYPE_CONTROLLER_MAC_ADDR 0x0009
#define MSGELEMTYPE_AP_NAME 0x000a
#define MSGELEMTYPE_AP_DESCRIPTOR 0x000b
#define MSGELEMTYPE_AP_IP_ADDR 0x000c
#define MSGELEMTYPE_AP_MAC_ADDR 0x000d
//#define MSGELEMTYPE_RETURNED_MSGELEM 0x000e
#define MSGELEMTYPE_CONTROLLER_NEXTSEQ 0x0010
#define MSGELEMTYPE_DESIRED_CONF_LIST 0x0011
#define MSGELEMTYPE_SSID 0x0101
#define MSGELEMTYPE_CHANNEL 0x0102
#define MSGELEMTYPE_HARDWARE_MODE 0x0103
#define MSGELEMTYPE_SUPPRESS_SSID 0x0104
#define MSGELEMTYPE_SECURITY_OPTION 0x0105
#define MSGELEMTYPE_MACFILTER_MODE 0x0106
#define MSGELEMTYPE_MACFILTER_LIST 0x0107
#define MSGELEMTYPE_TX_POWER 0x0108
#define MSGELEMTYPE_WPA_PWD 0x0202
#define MSGELEMTYPE_ADD_MACFILTER 0x0501
#define MSGELEMTYPE_DEL_MACFILTER 0x0502
#define MSGELEMTYPE_CLEAR_MACFILTER 0x0503
#define MSGELEMTYPE_RESET_MACFILTER 0x0504
#define MSGELEMTYPE_SYSTEM_COMMAND 0x0401
//#define MSGELEMTYPE_SCANNED_WLAN_INFO 0x0402
//#define MSGELEMTYPE_STATION_INFO 0x0403
//#define MSGELEMTYPE_STATISTICS 0x0404

/* result code */
#define RESULT_SUCCESS 0x0000
#define RESULT_FAILURE 0x0001
//#define RESULT_UNRECOGNIZED_REQ 0x0002

/* reason code */
#define REASON_INVALID_VERSION 0x0101
#define REASON_INSUFFICIENT_RESOURCE 0x0102

/* registered service */
#define REGISTERED_SERVICE_CONF_STA 0x00

/* discovery type */
#define DISCOVERY_TPYE_DISCOVERY 0
#define DISCOVERY_TPYE_STATIC 1
#define DISCOVERY_TPYE_DEFAULT_GATE 2

/* returned msg elem */
//#define RETERNED_ELEM_UNRECOGNIZED_ELEM 0x0000

/* hardware mode */
#define HWMODE_A 0
#define HWMODE_B 1
#define HWMODE_G 2
#define HWMODE_N 3

/* suppress ssid */
#define SUPPRESS_SSID_DISABLED 0
#define SUPPRESS_SSID_ENABLED 1

/* security option */
#define SECURITY_OPEN 0
//#define SECURITY_WEP 1
#define SECURITY_WPA_WPA2_MIXED 2
#define SECURITY_WPA 3
#define SECURITY_WPA2 4

/* mac filter */
#define FILTER_NONE 0
#define FILTER_ACCEPT_ONLY 1
#define FILTER_DENY 2

/* system command */
#define SYSTEM_WLAN_DOWN 0
#define SYSTEM_WLAN_UP 1
#define SYSTEM_WLAN_RESTART 2
#define SYSTEM_NETWORK_RESTART 3
//#define SYSTEM_SYSTEM_RESTART 4

/* fsm */
typedef enum {
	ENTER_DOWN,
	ENTER_DISCOVERY,
	ENTER_REGISTER,
	ENTER_RUN
} ap_state;

/**
 * initialize protocol_msg structure, without any msg
 * after an protocol_msg being created, init_protocol_msg or init_protocol_msg_size must be called immediately
 * @param  mess  [a not initialized value]
 */
#define		init_protocol_msg(mess) {\
							(mess).msg = NULL;\
							(mess).offset = 0;\
							(mess).type = 0; }

/**
 * initialize protocol_msg structure, including memory allocation and clear, for required size
 * after an protocol_msg being created, init_protocol_msg or init_protocol_msg_size must be called immediately
 * @param  mess  [a not initialized value]
 * @param  size  [required size]
 */
#define		init_protocol_msg_size(mess, size, err) {\
							create_object(((mess).msg), (size), err);\
							zero_memory(((mess).msg), (size));\
							(mess).offset = 0;\
							(mess).type = 0; }

/**
 * free protocol_msg structure, including releasing memory
 * before an protocol_msg, that is local value, being destoryed, this function must be called
 * @param  mess  [value that need to be freed]
 */
#define		free_protocol_msg(mess) {\
 							free_object(((mess).msg));\
							(mess).msg = NULL;\
							(mess).offset = 0;\
							(mess).type = 0; }

/**
 * create a array of protocol_msg structure, but not include memory allocation. commonly used in msg element
 * @param  ar_name  [name for array]
 * @param  ar_size  [required size for array]
 */
#define 	create_protocol_arr(ar_name, ar_size, err) {\
							create_array(ar_name, ar_size, protocol_msg, err)\
							int i;\
							for(i=0;i<(ar_size); i++) {\
								(ar_name)[i].msg = NULL;\
								(ar_name)[i].offset = 0;\
							} }

/**
 * free a array of protocol_msg structure, it's worth mentioning that this action includes releasing memory
 * @param  ar_name  [name for array]
 * @param  ar_size  [size of array]
 */
#define 	free_arr_and_protocol_msg(ar_name, ar_size) {\
							int i;\
							for(i=0;i<(ar_size); i++) {\
								free_protocol_msg((ar_name)[i]);\
							}\
							free_object(ar_name); }

bool assemble_msg(protocol_msg *msg_p, u16 apid, u32 seq_num,
						 u16 msg_type, protocol_msg *msgelems, int msgelems_num);

void copy_msg(protocol_msg *msg_p, protocol_msg *from_p);

bool parse_header(protocol_msg *msg_p, header_val *valPtr);
void parse_msgelem(protocol_msg *msg_p, u16 *type, u16 *len);
void parse_unrecognized_msgelem(protocol_msg *msg_p, int len);
void parse_repeated_msgelem(protocol_msg *msg_p, int len);


bool parse_controller_name(protocol_msg *msg_p, int len, char **valPtr);
bool parse_controller_desc(protocol_msg *msg_p, int len, char **valPtr);
bool parse_controller_ip(protocol_msg *msg_p, int len, u32 *valPtr);
bool parse_controller_mac(protocol_msg *msg_p, int len, u8 *valPtr);
bool parse_controller_nextseq(protocol_msg *msg_p, int len, u32 *valPtr);

bool assemble_register_service(protocol_msg *msg_p);
bool assemble_ap_name(protocol_msg *msg_p);
bool assemble_ap_desc(protocol_msg *msg_p);
bool assemble_ap_ip(protocol_msg *msg_p);
bool assemble_ap_mac(protocol_msg *msg_p);
bool assemble_ap_discovery_type(protocol_msg *msg_p);

bool parse_res_code(protocol_msg *msg_p, int len, u16 *valPtr);
bool parse_reason_code(protocol_msg *msg_p, int len, u16 *valPtr);
bool parse_assigned_apid(protocol_msg *msg_p, int len, u16 *valPtr);
bool parse_register_service(protocol_msg *msg_p, int len, u8 *valPtr);

bool assemble_ssid(protocol_msg *msg_p);
bool assemble_channel(protocol_msg *msg_p);
bool assemble_hwmode(protocol_msg *msg_p);
bool assemble_hide_ssid(protocol_msg *msg_p);
bool assemble_sec_opt(protocol_msg *msg_p);
bool assemble_macfilter_mode(protocol_msg *msg_p);
bool assemble_macfilter_list(protocol_msg *msg_p);
bool assemble_tx_power(protocol_msg *msg_p);
bool assemble_wpa_pwd(protocol_msg *msg_p);

bool parse_desired_conf_list(protocol_msg *msg_p, int len, u8 **valPtr);
bool parse_ssid(protocol_msg *msg_p, int len, char **valPtr);
bool parse_channel(protocol_msg *msg_p, int len, u8 *valPtr);
bool parse_hwmode(protocol_msg *msg_p, int len, u8 *valPtr);
bool parse_hide_ssid(protocol_msg *msg_p, int len, u8 *valPtr);
bool parse_sec_opt(protocol_msg *msg_p, int len, u8 *valPtr);
bool parse_macfilter_mode(protocol_msg *msg_p, int len, u8 *valPtr);
bool parse_tx_power(protocol_msg *msg_p, int len, u8 *valPtr);
bool parse_wpa_pwd(protocol_msg *msg_p, int len, char **valPtr);
bool parse_mac_list(protocol_msg *msg_p, int len, char ***valPtr);
bool parse_sys_cmd(protocol_msg *msg_p, int len, u8 *valPtr);

#endif
