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
	APLog("AP is down");
	APCloseLogFile();
}

int main()
{
	APStateTransition nextState = AP_ENTER_DISCOVERY;

	APInitProtocol();
	APDefaultSettings();

	if(!APInitErrorHandling()) {
		APErrorLog("Can't init error handling structure");
		exit(1);
	};

	APLog("Start reading the setting file");
	if(!APParseSettingsFile()) {
		APErrorLog("Can't read setting file");
		exit(1);
	}

	// if(!APCheckSettings()) {
	// 	APErrorLog("There may exist illegal assignment");
	// 	exit(1);
	// }

	APInitLogFile();
	APLog("Finished reading the setting file");
	
	// if(!APNetworkInitIfname()) {
	// 	APErrorLog("Can't obtain Ethernet interface");
	// 	//APErrorLog("Can't obtain Ethernet or Wlan interface name");
	// 	exit(1);
	// }
	if(gIfEthName == NULL) {
		APErrorLog("Can't obtain Ethernet interface");
		exit(1);
	}

	if(!APNetworkInitLocalAddr(&gAPIPAddr, gAPMACAddr, &gAPDefaultGateway)) {
		APErrorLog("Can't init local address");
		exit(1);
	}

	APDebugLog(5, "Local IP: %u.%u.%u.%u", (u8)(gAPIPAddr >> 24), (u8)(gAPIPAddr >> 16),\
	  (u8)(gAPIPAddr >> 8),  (u8)(gAPIPAddr >> 0));
	APDebugLog(5, "Local MAC: %02x:%02x:%02x:%02x:%02x:%02x", gAPMACAddr[0], gAPMACAddr[1],\
	 gAPMACAddr[2], gAPMACAddr[3], gAPMACAddr[4], gAPMACAddr[5]);
	APDebugLog(5, "Local gateway: %u.%u.%u.%u", (u8)(gAPDefaultGateway >> 24), (u8)(gAPDefaultGateway >> 16),\
	  (u8)(gAPDefaultGateway >> 8),  (u8)(gAPDefaultGateway >> 0));

	APDebugLog(3, "All init is finished");
	APLog("Starting AP...");

	/* if Controller address is given, jump Discovery and use this address for register */
	if(gDiscoveryType != DISCOVERY_TPYE_DISCOVERY) 
	{
		switch(gDiscoveryType) {
			case DISCOVERY_TPYE_STATIC:
				if(gStaticControllerIPAddr != 0) {
					gControllerIPAddr = gStaticControllerIPAddr;
					nextState = AP_ENTER_REGISTER;
					APLog("Use static controller addr to register: %u.%u.%u.%u", (u8)(gControllerIPAddr >> 24),\
		 				(u8)(gControllerIPAddr >> 16),  (u8)(gControllerIPAddr >> 8),  (u8)(gControllerIPAddr >> 0));
				} else {
					APLog("Static address is not specified");
					gDiscoveryType = DISCOVERY_TPYE_DISCOVERY;
				}
				break;
			case DISCOVERY_TPYE_DEFAULT_GATE:
				if(gAPDefaultGateway != 0) {
				 	gControllerIPAddr = gAPDefaultGateway;
					nextState = AP_ENTER_REGISTER;
					APLog("Use default gateway addr to register: %u.%u.%u.%u", (u8)(gControllerIPAddr >> 24),\
		 				(u8)(gControllerIPAddr >> 16),  (u8)(gControllerIPAddr >> 8),  (u8)(gControllerIPAddr >> 0));
			 	} else {
					APLog("Default gateway is not set");
					gDiscoveryType = DISCOVERY_TPYE_DISCOVERY;
				}
			 //case DISCOVERY_TPYE_DNS:
			 default:
			 	break;
		}
	}
	

	/* start acamp state machine */	
	AP_REPEAT_FOREVER {
		switch(nextState) {
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
				// nextState = APEnterDown();
				APDestroy();
				return 0;
				break;
		}
	}

	return 0;
}
