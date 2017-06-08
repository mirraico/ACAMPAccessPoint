#ifndef __AP_H__
#define __AP_H__

#include "common.h"

extern u32 ap_seqnum;
extern u32 controller_seqnum;

extern int retransmit_interval;
extern int keepalive_interval;
extern int max_retransmit;

extern u16 ap_apid;
extern u8 ap_discovery_type;
extern u8 ap_register_service;

extern char* ap_ethname;
extern u32 static_controller_ip; 
extern u32 controller_ip; 
extern u8 controller_mac[6];
extern u32 ap_ip;
extern u8 ap_mac[6];
extern u32 ap_default_gw; //unused

extern char* controller_name;
extern char* controller_desc;
extern char* ap_name;
extern char* ap_desc;

/* all wireless conf */
extern struct wlconf *wlconf;
extern bool wlflag;

#define ap_seqnum_inc()         ap_seqnum++
#define controller_seqnum_inc()         controller_seqnum++

void init_protocol();


#endif // AP_H