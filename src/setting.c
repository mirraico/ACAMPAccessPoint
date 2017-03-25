#include "setting.h"

FILE* gSettingsFile = NULL;

/**
 * get one "useful" (not a comment, not blank) line from the config file
 * @param	configFile [config file]
 * @return	[a "useful" line]
 */
char * APGetLine(FILE *configFile) {

	char *buff = NULL;
	char *command = NULL;
	char *ret = NULL;

	AP_CREATE_STRING_SIZE_ERR(buff, AP_BUFFER_SIZE, return NULL;);
	
	while ( ((ret = fgets(buff, AP_BUFFER_SIZE, configFile)) != NULL) &&\
		(buff[0] == '\n' || buff[0] == '\r' || buff[0] == '#') );
	
	if(buff != NULL && ret != NULL) {

		int len = strlen(buff);
		buff[len-1] = '\0';
		
		AP_CREATE_STRING_SIZE_ERR(command, len, return NULL;);
		strcpy(command, buff);
	}
	
	AP_FREE_OBJECT(buff);
	
	return command;
}

/**
 * get one "useful" (not blank) tag from a specified tag
 * @param	tag [a specified tag]
 * @return	[a "useful" tag]
 */
char* APExtractTag(char *tag)
{
	int i = strlen(tag) - 1;
	while(tag[i] == ' ' || tag[i] == '\t' || tag[i] == '\n' || tag[i] == '\r')
	{
		tag[i--] = '\0';
	}
	i = 0;
	while(tag[i] == ' ' || tag[i] == '\t' || tag[i] == '\n' || tag[i] == '\r')
	{
		i++;
	}
	return tag + i;
}

/**
 * get one "useful" (not blank) string value from a specified string
 * @param	str [a specified string]
 * @return	[a "useful" string vaule]
 */
char* APExtractStringVaule(char *str)
{
	int i = strlen(str) - 1;
	while(str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r')
	{
		str[i--] = '\0';
	}
	if(str[i] == '\"') str[i] = '\0';
	i = 0;
	while(str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r')
	{
		i++;
	}
	if(str[i] == '\"') i++;
	return str + i;
}

/**
 * get one "useful" (not blank) int value from a specified string
 * @param	str [a specified string]
 * @return	[a "useful" int vaule]
 */
int APExtractIntVaule(char *str)
{
	int i = strlen(str) - 1;
	while(str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r')
	{
		str[i--] = '\0';
	}
	i = 0;
	while(str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r')
	{
		i++;
	}
	return atoi(str + i);
}

/**
 * parse ap name, descriptor, and all configuration from files
 */
APBool APParseSettingsFile()
{
	char *line = NULL;

	gSettingsFile = fopen (AP_SETTINGS_FILE, "rb");
	if (gSettingsFile == NULL) {
		return AP_FALSE;
	}
	while((line = (char*)APGetLine(gSettingsFile)) != NULL) 
	{
		char *pos = NULL;

		if((pos=strchr (line, '='))==NULL) 
		{
			AP_FREE_OBJECT(line);
			continue;
		}
		pos[0] = '\0';


		if (!strcmp(APExtractTag(line), "AP_LOG_PATH"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			AP_CREATE_STRING_SIZE_ERR(gAPLogFileName, len + 1, return AP_FALSE;);
			AP_ZERO_MEMORY(gAPLogFileName, len + 1);
			AP_COPY_MEMORY(gAPLogFileName, value, len);

			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "AP_LOG_LEVEL"))
		{
			gAPLogLevel = APExtractIntVaule(pos+1);
			
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "AP_STDOUT_LOG_LEVEL"))
		{
			gAPStdoutLevel = APExtractIntVaule(pos+1);
			
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "AP_NAME"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			AP_CREATE_STRING_SIZE_ERR(gAPName, len + 1, return AP_FALSE;);
			AP_ZERO_MEMORY(gAPName, len + 1);
			AP_COPY_MEMORY(gAPName, value, len);
			
			APDebugLog(5, "CONF AP Name: %s", gAPName);
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "AP_DESCRIPTOR"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			AP_CREATE_STRING_SIZE_ERR(gAPDescriptor, len + 1, return AP_FALSE;);
			AP_ZERO_MEMORY(gAPDescriptor, len + 1);
			AP_COPY_MEMORY(gAPDescriptor, value, len);

			APDebugLog(5, "CONF AP Descriptor: %s", gAPDescriptor);
			AP_FREE_OBJECT(line);
			continue;
		}
		if (!strcmp(APExtractTag(line), "ETH_INTERFACE"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			AP_CREATE_STRING_SIZE_ERR(gIfEthName, len + 1, return AP_FALSE;);
			AP_ZERO_MEMORY(gIfEthName, len + 1);
			AP_COPY_MEMORY(gIfEthName, value, len);

			APDebugLog(5, "CONF Eth Interface Name: %s", gIfEthName);
			AP_FREE_OBJECT(line);
			continue;
		}
		if (!strcmp(APExtractTag(line), "DISCOVERY_TYPE"))
		{
			gDiscoveryType = APExtractIntVaule(pos+1);

			APDebugLog(5, "CONF Discovery Type: %u", gDiscoveryType);
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "STATIC_CONTROLLER_ADDRESS"))
		{
			char* ip;
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			AP_CREATE_STRING_SIZE_ERR(ip, len + 1, return AP_FALSE;);
			AP_ZERO_MEMORY(ip, len + 1);
			AP_COPY_MEMORY(ip, value, len);
			gStaticControllerIPAddr = ntohl(inet_addr(ip));
			AP_FREE_OBJECT(ip);

			APDebugLog(5, "CONF Static Controller IP Addr: %u.%u.%u.%u", (u8)(gStaticControllerIPAddr >> 24), (u8)(gStaticControllerIPAddr >> 16),\
	  			(u8)(gStaticControllerIPAddr >> 8),  (u8)(gStaticControllerIPAddr >> 0));
			AP_FREE_OBJECT(line);
			continue;
		}
		if (!strcmp(APExtractTag(line), "REGISTERED_SERVICE"))
		{
			gRegisteredService = APExtractIntVaule(pos+1);
			
			APDebugLog(5, "CONF Registered Service: %u", gRegisteredService);
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "WLAN"))
		{
			int flag = APExtractIntVaule(pos+1);
			
			if(!flag) wlflag = AP_FALSE;
			
			APDebugLog(5, "CONF MAC Filter: Reset");
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "SSID"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			char *ssid;
			AP_CREATE_STRING_SIZE_ERR(ssid, len + 1, return AP_FALSE;);
			AP_ZERO_MEMORY(ssid, len + 1);
			AP_COPY_MEMORY(ssid, value, len);
			wlconf->set_ssid(wlconf, ssid);

			APDebugLog(5, "CONF SSID: %s", ssid);
			AP_FREE_OBJECT(ssid);
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "SUPPRESS_SSID"))
		{
			int suppressSSID = APExtractIntVaule(pos+1);

			if(suppressSSID == SUPPRESS_SSID_ENABLED) {
				wlconf->set_ssid_hidden(wlconf, true);
			} else {
				wlconf->set_ssid_hidden(wlconf, false);
			}
			
			APDebugLog(5, "CONF Suppress SSID: %u", suppressSSID);
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "HW_MODE"))
		{
			int hwMode = APExtractIntVaule(pos+1);
			switch(hwMode)
			{
				case HWMODE_A:
					wlconf->set_hwmode(wlconf, ONLY_A);
					break;
				case HWMODE_B:
					wlconf->set_hwmode(wlconf, ONLY_B);
					break;
				case HWMODE_G:
					wlconf->set_hwmode(wlconf, ONLY_G);
					break;
				case HWMODE_N:
					wlconf->set_hwmode(wlconf, ONLY_N);
					break;
			}

			APDebugLog(5, "CONF Hardware Mode: %u", hwMode);
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "CHANNEL"))
		{
			int channel = APExtractIntVaule(pos+1);
			
			wlconf->set_channel(wlconf, channel);
			
			APDebugLog(5, "CONF Channel: %d", channel);
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "TX_POWER"))
		{
			int tx_power = APExtractIntVaule(pos+1);
			
			wlconf->set_txpower(wlconf, tx_power);
			
			APDebugLog(5, "CONF Tx Power: %d", tx_power);
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "SECURITY_OPTION"))
		{
			int securityOption = APExtractIntVaule(pos+1);
			
			switch(securityOption)
			{
				case SECURITY_OPEN:
					wlconf->set_encryption(wlconf, NO_ENCRYPTION);
					break;
				case SECURITY_WPA_WPA2_MIXED:
					wlconf->set_encryption(wlconf, WPA_WPA2_MIXED);
					break;
				case SECURITY_WPA:
					wlconf->set_encryption(wlconf, WPA_PSK);
					break;
				case SECURITY_WPA2:
					wlconf->set_encryption(wlconf, WPA2_PSK);
					break;
			}

			APDebugLog(5, "CONF Security Option: %u", securityOption);
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "WPA_PASSPHRASE"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			char *password;
			AP_CREATE_STRING_SIZE_ERR(password, len + 1, return AP_FALSE;);
			AP_ZERO_MEMORY(password, len + 1);
			AP_COPY_MEMORY(password, value, len);
			wlconf->set_key(wlconf, password);

			APDebugLog(5, "CONF WPA Password: %s", password);
			AP_FREE_OBJECT(password);
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "RESET_MAC_FILTER"))
		{
			int reset = APExtractIntVaule(pos+1);
			
			if(reset) wlconf->set_macfilter(wlconf, MAC_FILTER_NONE);
			
			APDebugLog(5, "CONF MAC Filter: Reset");
			AP_FREE_OBJECT(line);
			continue;	
		}
		/*
		if (!strcmp(APExtractTag(line), "WEP_DEFAULT_KEY"))
		{
			gWEP.default_key = APExtractIntVaule(pos+1);
			
			APDebugLog(5, "CONF WEP Default Key: %u", gWEP.default_key);
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "WEP_KEY0"))
		{
			int keylen = strlen(pos+1);
			if(keylen == 7 || keylen == 15 || keylen == 18 || keylen == 10 || keylen == 26 || keylen == 32) {
				char* value = APExtractStringVaule(pos+1);
				int len = strlen(value);
				
				AP_CREATE_STRING_SIZE_ERR(gWEP.key0, len + 1, return AP_FALSE;);
				AP_ZERO_MEMORY(gWEP.key0, len + 1);
				AP_COPY_MEMORY(gWEP.key0, value, len);
				
				if(keylen == 7) {
					gWEP.key0_type = WEPTYPE_CHAR5;
					APDebugLog(5, "CONF WEP key0: %s (Type CHAR5)", gWEP.key0);
				} else if(keylen == 15) {
					gWEP.key0_type = WEPTYPE_CHAR13;
					APDebugLog(5, "CONF WEP key0: %s (Type CHAR13)", gWEP.key0);
				} else if(keylen == 18) {
					gWEP.key0_type = WEPTYPE_CHAR16;
					APDebugLog(5, "CONF WEP key0: %s (Type CHAR16)", gWEP.key0);
				} else if(keylen == 10) {
					gWEP.key0_type = WEPTYPE_HEX10;
					APDebugLog(5, "CONF WEP key0: %s (Type HEX10)", gWEP.key0);
				} else if(keylen == 26) {
					gWEP.key0_type = WEPTYPE_HEX26;
					APDebugLog(5, "CONF WEP key0: %s (Type HEX26)", gWEP.key0);
				} else if(keylen == 32) {
					gWEP.key0_type = WEPTYPE_HEX32;
					APDebugLog(5, "CONF WEP key0: %s (Type HEX32)", gWEP.key0);
				}
			}
		}
		if (!strcmp(APExtractTag(line), "WEP_KEY1"))
		{
			int keylen = strlen(pos+1);
			if(keylen == 7 || keylen == 15 || keylen == 18 || keylen == 10 || keylen == 26 || keylen == 32) {
				char* value = APExtractStringVaule(pos+1);
				int len = strlen(value);
				
				AP_CREATE_STRING_SIZE_ERR(gWEP.key1, len + 1, return AP_FALSE;);
				AP_ZERO_MEMORY(gWEP.key1, len + 1);
				AP_COPY_MEMORY(gWEP.key1, value, len);
				
				if(keylen == 7) {
					gWEP.key1_type = WEPTYPE_CHAR5;
					APDebugLog(5, "CONF WEP key1: %s (Type CHAR5)", gWEP.key1);
				} else if(keylen == 15) {
					gWEP.key1_type = WEPTYPE_CHAR13;
					APDebugLog(5, "CONF WEP key1: %s (Type CHAR13)", gWEP.key1);
				} else if(keylen == 18) {
					gWEP.key1_type = WEPTYPE_CHAR16;
					APDebugLog(5, "CONF WEP key1: %s (Type CHAR16)", gWEP.key1);
				} else if(keylen == 10) {
					gWEP.key1_type = WEPTYPE_HEX10;
					APDebugLog(5, "CONF WEP key1: %s (Type HEX10)", gWEP.key1);
				} else if(keylen == 26) {
					gWEP.key1_type = WEPTYPE_HEX26;
					APDebugLog(5, "CONF WEP key1: %s (Type HEX26)", gWEP.key1);
				} else if(keylen == 32) {
					gWEP.key1_type = WEPTYPE_HEX32;
					APDebugLog(5, "CONF WEP key1: %s (Type HEX32)", gWEP.key1);
				}
			}
		}
		if (!strcmp(APExtractTag(line), "WEP_KEY2"))
		{
			int keylen = strlen(pos+1);
			if(keylen == 7 || keylen == 15 || keylen == 18 || keylen == 10 || keylen == 26 || keylen == 32) {
				char* value = APExtractStringVaule(pos+1);
				int len = strlen(value);
				
				AP_CREATE_STRING_SIZE_ERR(gWEP.key2, len + 1, return AP_FALSE;);
				AP_ZERO_MEMORY(gWEP.key2, len + 1);
				AP_COPY_MEMORY(gWEP.key2, value, len);
				
				if(keylen == 7) {
					gWEP.key2_type = WEPTYPE_CHAR5;
					APDebugLog(5, "CONF WEP key2: %s (Type 1CHAR5)", gWEP.key2);
				} else if(keylen == 15) {
					gWEP.key2_type = WEPTYPE_CHAR13;
					APDebugLog(5, "CONF WEP key2: %s (Type CHAR13)", gWEP.key2);
				} else if(keylen == 18) {
					gWEP.key2_type = WEPTYPE_CHAR16;
					APDebugLog(5, "CONF WEP key2: %s (Type CHAR16)", gWEP.key2);
				} else if(keylen == 10) {
					gWEP.key2_type = WEPTYPE_HEX10;
					APDebugLog(5, "CONF WEP key2: %s (Type HEX10)", gWEP.key2);
				} else if(keylen == 26) {
					gWEP.key2_type = WEPTYPE_HEX26;
					APDebugLog(5, "CONF WEP key2: %s (Type HEX26)", gWEP.key2);
				} else if(keylen == 32) {
					gWEP.key2_type = WEPTYPE_HEX32;
					APDebugLog(5, "CONF WEP key2: %s (Type HEX32)", gWEP.key2);
				}
			}
		}
		if (!strcmp(APExtractTag(line), "WEP_KEY3"))
		{
			int keylen = strlen(pos+1);
			if(keylen == 7 || keylen == 15 || keylen == 18 || keylen == 10 || keylen == 26 || keylen == 32) {
				char* value = APExtractStringVaule(pos+1);
				int len = strlen(value);
				
				AP_CREATE_STRING_SIZE_ERR(gWEP.key3, len + 1, return AP_FALSE;);
				AP_ZERO_MEMORY(gWEP.key3, len + 1);
				AP_COPY_MEMORY(gWEP.key3, value, len);
				
				if(keylen == 7) {
					gWEP.key3_type = WEPTYPE_CHAR5;
					APDebugLog(5, "CONF WEP key3: %s (Type CHAR5)", gWEP.key3);
				} else if(keylen == 15) {
					gWEP.key3_type = WEPTYPE_CHAR13;
					APDebugLog(5, "CONF WEP key3: %s (Type CHAR13)", gWEP.key3);
				} else if(keylen == 18) {
					gWEP.key3_type = WEPTYPE_CHAR16;
					APDebugLog(5, "CONF WEP key3: %s (Type CHAR16)", gWEP.key3);
				} else if(keylen == 10) {
					gWEP.key3_type = WEPTYPE_HEX10;
					APDebugLog(5, "CONF WEP key3: %s (Type HEX10)", gWEP.key3);
				} else if(keylen == 26) {
					gWEP.key3_type = WEPTYPE_HEX26;
					APDebugLog(5, "CONF WEP key3: %s (Type HEX26)", gWEP.key3);
				} else if(keylen == 32) {
					gWEP.key3_type = WEPTYPE_HEX32;
					APDebugLog(5, "CONF WEP key3: %s (Type HEX32)", gWEP.key3);
				}
			}
		}
		if (!strcmp(APExtractTag(line), "WPA_VERSION"))
		{
			gWPA.version = APExtractIntVaule(pos+1);
			
			APDebugLog(5, "CONF WPA Version: %u", gWPA.version);
			AP_FREE_OBJECT(line);
			continue;	
		}
		*/
		/*
		if (!strcmp(APExtractTag(line), "WPA_PAIRWIRE"))
		{
			gWPA.pairwire_cipher = APExtractIntVaule(pos+1);
			
			APDebugLog(5, "CONF WPA Pairwire Cipher: %u", gWPA.pairwire_cipher);
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "WPA_GROUP_REKEY"))
		{
			gWPA.group_rekey = APExtractIntVaule(pos+1);
			
			APDebugLog(5, "CONF WPA Group Rekey: %u", gWPA.group_rekey);
			AP_FREE_OBJECT(line);
			continue;	
		}
		*/
		
		AP_FREE_OBJECT(line);
		continue;	
	}
	return AP_TRUE;
}

// APBool APCheckSettings()
// {
// 	if(gDiscoveryType < 0 || gDiscoveryType > 3) {
// 		APErrorLog("Invalid Discovery Type"); return AP_FALSE;
// 	}
// 	if(gRegisteredService < 0 || gRegisteredService > 0) {
// 		APErrorLog("Invalid Registered Service"); return AP_FALSE;
// 	}
// 	if(strlen(gAPName) < 4 || strlen(gAPName) > 32) {
// 		APErrorLog("Invalid AP Name"); return AP_FALSE;
// 	}
// 	if(strlen(gAPDescriptor) < 1 || strlen(gAPDescriptor) > 128) {
// 		APErrorLog("Invalid AP Descriptor"); return AP_FALSE;
// 	}
// 	if(strlen(gSSID) < 1 || strlen(gSSID) > 32) {
// 		APErrorLog("Invalid SSID"); return AP_FALSE;
// 	}
// 	if(gChannel < 1 || gChannel > 13) {
// 		APErrorLog("Invalid Channel"); return AP_FALSE;
// 	}
// 	if(gHardwareMode < 0 || gHardwareMode > 3) {
// 		APErrorLog("Invalid Hardware Mode"); return AP_FALSE;
// 	}
// 	if(gSuppressSSID < 0 || gSuppressSSID > 2) {
// 		APErrorLog("Invalid Suppress SSID"); return AP_FALSE;
// 	}
// 	if(gSecurityOption < 0 || gSecurityOption > 3) {
// 		APErrorLog("Invalid Security Option"); return AP_FALSE;
// 	}

// 	if(gDiscoveryType == 1 && gStaticControllerIPAddr == 0) {
// 		APErrorLog("Invalid Static Controller IP Addr"); return AP_FALSE;
// 	}

// 	return AP_TRUE;
// }


void APDefaultSettings()
{
	int i;

	gAPLogFileName = NULL; //no file log
	gAPLogLevel = 0;
	gAPStdoutLevel = 0;

	gAPName = "unnamed AP";
	gAPDescriptor = "no descriptor";

	/* init in APNetworkInitIfname() */
	gIfEthName = "br-lan";
	//gIfWlanName = NULL;
	wlflag = AP_TRUE;

	/* IP, MAC and default gateway addr will be automatically obtained soon by APNetworkInitLocalAddr() */
	gAPIPAddr = 0;
	for(i = 0; i < 6; i++) {
		gAPMACAddr[i] = 0;
	}
	gAPDefaultGateway = 0;

    gDiscoveryType = DISCOVERY_TPYE_DISCOVERY; //broadcast discovery
	gStaticControllerIPAddr = 0; //static Controller IP addr
	gRegisteredService = REGISTERED_SERVICE_CONF_STA; //configuration and station service

	gControllerName = NULL;
	gControllerDescriptor = NULL;
	gControllerIPAddr = 0; 
	for(i = 0; i < 6; i++) {
		gControllerMACAddr[i] = 0;
	}

	// wlconf->set_ssid(wlconf, "openwrt1");
	// wlconf->set_ssid_hidden(wlconf, false);
	// wlconf->set_channel(wlconf, 7);
	// wlconf->set_hwmode(wlconf, ONLY_G);
	// wlconf->set_encryption(wlconf, NO_ENCRYPTION);
	// wlconf->set_txpower(wlconf, 18);
	// /* MAC Filter configuration */
	// wlconf->set_macfilter(wlconf, MAC_FILTER_NONE);


	// gSSID = "ap_ssid";
	// gSuppressSSID = SUPPRESS_SSID_DISABLED;
	// gHardwareMode = HWMODE_G;
	// gChannel = 7;
	// gSecurityOption = SECURITY_OPEN;


	// gWEP.default_key = 0;
	// gWEP.key0_type = 0;
	// gWEP.key0 = NULL;
	// gWEP.key1_type = 0;
	// gWEP.key1 = NULL;
	// gWEP.key2_type = 0;
	// gWEP.key2 = NULL;
	// gWEP.key3_type = 0;
	// gWEP.key3 = NULL;


	// gWPA.password = NULL;
	//gWPA.pairwire_cipher = WPA_PAIRWIRECIPHER_TKIP_CCMP;
}