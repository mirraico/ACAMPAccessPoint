#include "common.h"
#include "protocol.h"
#include "ap.h"
#include "setting.h"
#include "log.h"
#include "network.h"

void APDestroy()
{
	APDebugLog(3, "Start destroying AP");
	APNetworkCloseSocket(gSocket);
	APNetworkCloseSocket(gSocketBroad);
	wlconf_free(wlconf);
	APLog("AP is down");
	APCloseLogFile();
}

int main()
{
	APStateTransition nextState = AP_ENTER_DISCOVERY;

	wlconf = wlconf_alloc();

	init_protocol();
	APDefaultSettings();

	APLog("Start reading the setting file");
	if(!APParseSettingsFile()) {
		APErrorLog("Can't read setting file");
		exit(1);
	}

	wlconf->change_commit(wlconf);
	if(wlflag) system("wifi restart");
	else system("wifi down");

	APInitLogFile();
	APLog("Finished reading the setting file");
	
	if(ap_ethname == NULL) {
		APErrorLog("Can't obtain Ethernet interface");
		exit(1);
	}

	if(!APNetworkInitLocalAddr(&ap_ip, ap_mac, &ap_default_gw)) {
		APErrorLog("Can't init local address");
		exit(1);
	}

	APDebugLog(5, "Local IP: %u.%u.%u.%u", (u8)(ap_ip >> 24), (u8)(ap_ip >> 16),\
	  (u8)(ap_ip >> 8),  (u8)(ap_ip >> 0));
	APDebugLog(5, "Local MAC: %02x:%02x:%02x:%02x:%02x:%02x", ap_mac[0], ap_mac[1],\
	 ap_mac[2], ap_mac[3], ap_mac[4], ap_mac[5]);
	APDebugLog(5, "Local gateway: %u.%u.%u.%u", (u8)(ap_default_gw >> 24), (u8)(ap_default_gw >> 16),\
	  (u8)(ap_default_gw >> 8),  (u8)(ap_default_gw >> 0));

	APDebugLog(3, "All init is finished");
	APLog("Starting AP...");

	/* if Controller address is given, jump Discovery and use this address for register */
	if(ap_discovery_type != DISCOVERY_TPYE_DISCOVERY) 
	{
		switch(ap_discovery_type) {
			case DISCOVERY_TPYE_STATIC:
				if(static_controller_ip != 0) {
					controller_ip = static_controller_ip;
					nextState = AP_ENTER_REGISTER;
					APLog("Use static controller addr to register: %u.%u.%u.%u", (u8)(controller_ip >> 24),\
		 				(u8)(controller_ip >> 16),  (u8)(controller_ip >> 8),  (u8)(controller_ip >> 0));
				} else {
					APLog("Static address is not specified");
					ap_discovery_type = DISCOVERY_TPYE_DISCOVERY;
				}
				break;
			case DISCOVERY_TPYE_DEFAULT_GATE:
				if(ap_default_gw != 0) {
				 	controller_ip = ap_default_gw;
					nextState = AP_ENTER_REGISTER;
					APLog("Use default gateway addr to register: %u.%u.%u.%u", (u8)(controller_ip >> 24),\
		 				(u8)(controller_ip >> 16),  (u8)(controller_ip >> 8),  (u8)(controller_ip >> 0));
			 	} else {
					APLog("Default gateway is not set");
					ap_discovery_type = DISCOVERY_TPYE_DISCOVERY;
				}
			 default:
			 	break;
		}
	}
	

	/* start acamp state machine */	
	while(1) 
	{
		switch(nextState) 
		{
			case AP_ENTER_DISCOVERY:
				nextState = APEnterDiscovery();
				break;
			case AP_ENTER_REGISTER:
				nextState = APEnterRegister();
				break;
			case AP_ENTER_RUN:
				nextState = APEnterRun();
				break;
			case AP_ENTER_DOWN:
				APDestroy();
				return 0;
				break;
		}
	}

	return 0;
}
