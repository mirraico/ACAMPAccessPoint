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

	create_string(buff, BUFFER_SIZE, return NULL;);
	
	while ( ((ret = fgets(buff, BUFFER_SIZE, configFile)) != NULL) &&\
		(buff[0] == '\n' || buff[0] == '\r' || buff[0] == '#') );
	
	if(buff != NULL && ret != NULL) {

		int len = strlen(buff);
		buff[len-1] = '\0';
		
		create_string(command, len, return NULL;);
		strcpy(command, buff);
	}
	
	free_object(buff);
	
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
bool APParseSettingsFile()
{
	char *line = NULL;

	gSettingsFile = fopen (AP_SETTINGS_FILE, "rb");
	if (gSettingsFile == NULL) {
		return false;
	}
	while((line = (char*)APGetLine(gSettingsFile)) != NULL) 
	{
		char *pos = NULL;

		if((pos=strchr (line, '='))==NULL) 
		{
			free_object(line);
			continue;
		}
		pos[0] = '\0';


		if (!strcmp(APExtractTag(line), "AP_LOG_PATH"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			create_string(gAPLogFileName, len + 1, return false;);
			zero_memory(gAPLogFileName, len + 1);
			copy_memory(gAPLogFileName, value, len);

			free_object(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "AP_LOG_LEVEL"))
		{
			gAPLogLevel = APExtractIntVaule(pos+1);
			
			free_object(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "AP_STDOUT_LOG_LEVEL"))
		{
			gAPStdoutLevel = APExtractIntVaule(pos+1);
			
			free_object(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "AP_NAME"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			create_string(gAPName, len + 1, return false;);
			zero_memory(gAPName, len + 1);
			copy_memory(gAPName, value, len);
			
			APDebugLog(5, "CONF AP Name: %s", gAPName);
			free_object(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "AP_DESCRIPTOR"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			create_string(gAPDescriptor, len + 1, return false;);
			zero_memory(gAPDescriptor, len + 1);
			copy_memory(gAPDescriptor, value, len);

			APDebugLog(5, "CONF AP Descriptor: %s", gAPDescriptor);
			free_object(line);
			continue;
		}
		if (!strcmp(APExtractTag(line), "ETH_INTERFACE"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			create_string(gIfEthName, len + 1, return false;);
			zero_memory(gIfEthName, len + 1);
			copy_memory(gIfEthName, value, len);

			APDebugLog(5, "CONF Eth Interface Name: %s", gIfEthName);
			free_object(line);
			continue;
		}
		if (!strcmp(APExtractTag(line), "DISCOVERY_TYPE"))
		{
			gDiscoveryType = APExtractIntVaule(pos+1);

			APDebugLog(5, "CONF Discovery Type: %u", gDiscoveryType);
			free_object(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "STATIC_CONTROLLER_ADDRESS"))
		{
			char* ip;
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			create_string(ip, len + 1, return false;);
			zero_memory(ip, len + 1);
			copy_memory(ip, value, len);
			gStaticControllerIPAddr = ntohl(inet_addr(ip));
			free_object(ip);

			APDebugLog(5, "CONF Static Controller IP Addr: %u.%u.%u.%u", (u8)(gStaticControllerIPAddr >> 24), (u8)(gStaticControllerIPAddr >> 16),\
	  			(u8)(gStaticControllerIPAddr >> 8),  (u8)(gStaticControllerIPAddr >> 0));
			free_object(line);
			continue;
		}
		if (!strcmp(APExtractTag(line), "REGISTERED_SERVICE"))
		{
			gRegisteredService = APExtractIntVaule(pos+1);
			
			APDebugLog(5, "CONF Registered Service: %u", gRegisteredService);
			free_object(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "WLAN"))
		{
			int flag = APExtractIntVaule(pos+1);
			
			if(!flag) wlflag = false;
			
			APDebugLog(5, "CONF MAC Filter: Reset");
			free_object(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "SSID"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			char *ssid;
			create_string(ssid, len + 1, return false;);
			zero_memory(ssid, len + 1);
			copy_memory(ssid, value, len);
			wlconf->set_ssid(wlconf, ssid);

			APDebugLog(5, "CONF SSID: %s", ssid);
			free_object(ssid);
			free_object(line);
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
			free_object(line);
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
			free_object(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "CHANNEL"))
		{
			int channel = APExtractIntVaule(pos+1);
			
			wlconf->set_channel(wlconf, channel);
			
			APDebugLog(5, "CONF Channel: %d", channel);
			free_object(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "TX_POWER"))
		{
			int tx_power = APExtractIntVaule(pos+1);
			
			wlconf->set_txpower(wlconf, tx_power);
			
			APDebugLog(5, "CONF Tx Power: %d", tx_power);
			free_object(line);
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
			free_object(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "WPA_PASSPHRASE"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			char *password;
			create_string(password, len + 1, return false;);
			zero_memory(password, len + 1);
			copy_memory(password, value, len);
			wlconf->set_key(wlconf, password);

			APDebugLog(5, "CONF WPA Password: %s", password);
			free_object(password);
			free_object(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "RESET_MAC_FILTER"))
		{
			int reset = APExtractIntVaule(pos+1);
			
			if(reset) wlconf->set_macfilter(wlconf, MAC_FILTER_NONE);
			
			APDebugLog(5, "CONF MAC Filter: Reset");
			free_object(line);
			continue;	
		}
		
		free_object(line);
		continue;	
	}
	return true;
}

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
	wlflag = true;

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
}