#include "ap.h"

u32 ap_seqnum;
u32 controller_seqnum;

int retransmit_interval;
int keepalive_interval;
int max_retransmit;

u16 ap_apid;
u8 ap_discovery_type;
u8 ap_register_service;

char* ap_ethname;
u32 static_controller_ip; 
u32 controller_ip; 
u8 controller_mac[6];
u32 ap_ip;
u8 ap_mac[6];
u32 ap_default_gw; //unused

char* controller_name;
char* controller_des;
char* ap_name;
char* ap_des;

/* all wireless conf */
struct wlconf *wlconf;
bool wlflag;

/**
 * init ap, including seqnum, apid, etc, and set random seed
 */
void init_protocol()
{
	srand( (unsigned)time( NULL ) );

	ap_seqnum = rand();
	ap_apid = 0;

	retransmit_interval = 3;
	max_retransmit = 5;
	keepalive_interval = 30;
}

