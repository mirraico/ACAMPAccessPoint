#include "common.h"
#include "protocol.h"
#include "ap.h"
#include "setting.h"
#include "log.h"
#include "network.h"

void APDestroy()
{
	APDebugLog(3, "Start destroying AP");
	APCloseLogFile();
	APNetworkCloseSocket(gSocket);
	APNetworkCloseSocket(gSocketBroad);
}

int main()
{
	APStateTransition nextState = AP_ENTER_DISCOVERY;

	APInit();
	APDefaultSettings();

	if(!APInitErrorHandling()) {
		APErrorLog("Can't init error handling structure");
		exit(1);
	};

	if(!APParseSettingsFile()) {
		APErrorLog("Can't read setting file");
		exit(1);
	}

	APInitLogFile();
	APLog("Finish to read setting file");

	// APDebugLog(5, "gLogFileName: %s", gLogFileName);
	// APDebugLog(5, "gLogLevel: %d", gLogLevel);
	// APDebugLog(5, "gStdoutLevel: %d", gStdoutLevel);
	// APDebugLog(5, "gAPName: %s", gAPName);
	// APDebugLog(5, "gAPDescriptor: %s", gAPDescriptor);
	// APDebugLog(5, "gDiscoveryType: %d", gDiscoveryType);
	// APDebugLog(5, "gSSID: %s", gSSID);
	// APDebugLog(5, "gSuppressSSID: %d", gSuppressSSID);
	// APDebugLog(5, "gHardwareMode: %d", gHardwareMode);
	// APDebugLog(5, "gChannel: %d", gChannel);
	// APDebugLog(5, "gSecuritySetting: %d", gSecuritySetting);
	
	
	if(!APNetworkInitLocalAddr(&gAPIPAddr, gAPMACAddr, &gAPDefaultGateway)) {
		APErrorLog("Can't init local address");
		exit(1);
	}

	// APDebugLog(5, "Local IP:  %u.%u.%u.%u", (u8)(gAPIPAddr >> 24), (u8)(gAPIPAddr >> 16),\
	  (u8)(gAPIPAddr >> 8),  (u8)(gAPIPAddr >> 0));
	// APDebugLog(5, "Local MAC:  %02x:%02x:%02x:%02x:%02x:%02x", gAPMACAddr[0], gAPMACAddr[1],\
	 gAPMACAddr[2], gAPMACAddr[3], gAPMACAddr[4], gAPMACAddr[5]);
	// APDebugLog(5, "Local gateway:  %u.%u.%u.%u", (u8)(gAPDefaultGateway >> 24), (u8)(gAPDefaultGateway >> 16),\
	  (u8)(gAPDefaultGateway >> 8),  (u8)(gAPDefaultGateway >> 0));

	APDebugLog(3, "Finish all init");
	APLog("Starting WTP...");

	/* if Controller address is given, jump Discovery and use this address for register */
	// if(gDiscoveryType != 0) 
	// {
	// 	//todo: init single socket
	// 	nextState = AP_ENTER_REGISTER;
	// 	APDebugLog(3, "Use gived controller address:  %u.%u.%u.%u", (u8)(gControllerIPAddr >> 24),\
	// 	 (u8)(gControllerIPAddr >> 16),  (u8)(gControllerIPAddr >> 8),  (u8)(gControllerIPAddr >> 0));
	// }
	

	/* start acamp state machine */	
	AP_REPEAT_FOREVER {
		switch(nextState) {
			case AP_ENTER_DISCOVERY:
				nextState = APEnterDiscovery();
				break;
			case AP_ENTER_REGISTER:
				// nextState = APEnterRegister();
				break;
			case AP_ENTER_CONFIGURE:
				// nextState = APEnterConfigure();
				break;
			case AP_ENTER_CONFIGURE_RETRY:
				// nextState = APEnterConfigureRetry();
				break;
			case AP_ENTER_RUN:
				// nextState = APEnterRun();
				break;
			case AP_ENTER_DOWN:
				// nextState = APEnterDown();
				APDestroy();
				APLog("WTP is down");
				return 0;
				break;
		}
	}

	return 0;
}
