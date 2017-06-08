#include "setting.h"

static FILE* settings_file = NULL;

/**
 * get one "useful" (not a comment, not blank) line from the config file
 * @param	configFile [config file]
 * @return	[a "useful" line]
 */
char * get_line(FILE *configFile) {

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
char* extract_tag(char *tag)
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
char* extract_str(char *str)
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
int extract_int(char *str)
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
bool parse_settings_file()
{
	char *line = NULL;

	settings_file = fopen (SETTINGS_FILE, "rb");
	if (settings_file == NULL) {
		return false;
	}
	while((line = (char*)get_line(settings_file)) != NULL) 
	{
		char *pos = NULL;

		if((pos=strchr (line, '='))==NULL) 
		{
			free_object(line);
			continue;
		}
		pos[0] = '\0';


		if (!strcmp(extract_tag(line), "AP_LOG_PATH"))
		{
			char* value = extract_str(pos+1);
			int len = strlen(value);
			
			create_string(log_filename, len + 1, return false;);
			zero_memory(log_filename, len + 1);
			copy_memory(log_filename, value, len);

			free_object(line);
			continue;	
		}
		if (!strcmp(extract_tag(line), "AP_LOG_LEVEL"))
		{
			log_level = extract_int(pos+1);
			
			free_object(line);
			continue;	
		}
		if (!strcmp(extract_tag(line), "AP_STDOUT_LOG_LEVEL"))
		{
			log_stdlev = extract_int(pos+1);
			
			free_object(line);
			continue;	
		}
		if (!strcmp(extract_tag(line), "AP_NAME"))
		{
			char* value = extract_str(pos+1);
			int len = strlen(value);
			
			create_string(ap_name, len + 1, return false;);
			zero_memory(ap_name, len + 1);
			copy_memory(ap_name, value, len);
			
			log_d(5, "CONF AP Name: %s", ap_name);
			free_object(line);
			continue;	
		}
		if (!strcmp(extract_tag(line), "ap_descCRIPTOR"))
		{
			char* value = extract_str(pos+1);
			int len = strlen(value);
			
			create_string(ap_desc, len + 1, return false;);
			zero_memory(ap_desc, len + 1);
			copy_memory(ap_desc, value, len);

			log_d(5, "CONF AP Descriptor: %s", ap_desc);
			free_object(line);
			continue;
		}
		if (!strcmp(extract_tag(line), "ETH_INTERFACE"))
		{
			char* value = extract_str(pos+1);
			int len = strlen(value);
			
			create_string(ap_ethname, len + 1, return false;);
			zero_memory(ap_ethname, len + 1);
			copy_memory(ap_ethname, value, len);

			log_d(5, "CONF Eth Interface Name: %s", ap_ethname);
			free_object(line);
			continue;
		}
		if (!strcmp(extract_tag(line), "DISCOVERY_TYPE"))
		{
			ap_discovery_type = extract_int(pos+1);

			log_d(5, "CONF Discovery Type: %u", ap_discovery_type);
			free_object(line);
			continue;	
		}
		if (!strcmp(extract_tag(line), "STATIC_CONTROLLER_ADDRESS"))
		{
			char* ip;
			char* value = extract_str(pos+1);
			int len = strlen(value);
			
			create_string(ip, len + 1, return false;);
			zero_memory(ip, len + 1);
			copy_memory(ip, value, len);
			static_controller_ip = ntohl(inet_addr(ip));
			free_object(ip);

			log_d(5, "CONF Static Controller IP Addr: %u.%u.%u.%u", (u8)(static_controller_ip >> 24), (u8)(static_controller_ip >> 16),\
	  			(u8)(static_controller_ip >> 8),  (u8)(static_controller_ip >> 0));
			free_object(line);
			continue;
		}
		if (!strcmp(extract_tag(line), "REGISTERED_SERVICE"))
		{
			ap_register_service = extract_int(pos+1);
			
			log_d(5, "CONF Registered Service: %u", ap_register_service);
			free_object(line);
			continue;	
		}
		if (!strcmp(extract_tag(line), "WLAN"))
		{
			int flag = extract_int(pos+1);
			
			if(!flag) wlflag = false;
			
			log_d(5, "CONF MAC Filter: Reset");
			free_object(line);
			continue;	
		}
		if (!strcmp(extract_tag(line), "SSID"))
		{
			char* value = extract_str(pos+1);
			int len = strlen(value);
			
			char *ssid;
			create_string(ssid, len + 1, return false;);
			zero_memory(ssid, len + 1);
			copy_memory(ssid, value, len);
			wlconf->set_ssid(wlconf, ssid);

			log_d(5, "CONF SSID: %s", ssid);
			free_object(ssid);
			free_object(line);
			continue;	
		}
		if (!strcmp(extract_tag(line), "SUPPRESS_SSID"))
		{
			int suppressSSID = extract_int(pos+1);

			if(suppressSSID == SUPPRESS_SSID_ENABLED) {
				wlconf->set_ssid_hidden(wlconf, true);
			} else {
				wlconf->set_ssid_hidden(wlconf, false);
			}
			
			log_d(5, "CONF Suppress SSID: %u", suppressSSID);
			free_object(line);
			continue;	
		}
		if (!strcmp(extract_tag(line), "HW_MODE"))
		{
			int hwMode = extract_int(pos+1);
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

			log_d(5, "CONF Hardware Mode: %u", hwMode);
			free_object(line);
			continue;	
		}
		if (!strcmp(extract_tag(line), "CHANNEL"))
		{
			int channel = extract_int(pos+1);
			
			wlconf->set_channel(wlconf, channel);
			
			log_d(5, "CONF Channel: %d", channel);
			free_object(line);
			continue;	
		}
		if (!strcmp(extract_tag(line), "TX_POWER"))
		{
			int tx_power = extract_int(pos+1);
			
			wlconf->set_txpower(wlconf, tx_power);
			
			log_d(5, "CONF Tx Power: %d", tx_power);
			free_object(line);
			continue;	
		}
		if (!strcmp(extract_tag(line), "SECURITY_OPTION"))
		{
			int securityOption = extract_int(pos+1);
			
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

			log_d(5, "CONF Security Option: %u", securityOption);
			free_object(line);
			continue;	
		}
		if (!strcmp(extract_tag(line), "WPA_PASSPHRASE"))
		{
			char* value = extract_str(pos+1);
			int len = strlen(value);
			
			char *password;
			create_string(password, len + 1, return false;);
			zero_memory(password, len + 1);
			copy_memory(password, value, len);
			wlconf->set_key(wlconf, password);

			log_d(5, "CONF WPA Password: %s", password);
			free_object(password);
			free_object(line);
			continue;	
		}
		if (!strcmp(extract_tag(line), "RESET_MAC_FILTER"))
		{
			int reset = extract_int(pos+1);
			
			if(reset) wlconf->set_macfilter(wlconf, MAC_FILTER_NONE);
			
			log_d(5, "CONF MAC Filter: Reset");
			free_object(line);
			continue;	
		}
		
		free_object(line);
		continue;	
	}
	return true;
}

void init_default_settings()
{
	int i;

	log_filename = NULL; //no file log
	log_level = 0;
	log_stdlev = 0;

	ap_name = "unnamed AP";
	ap_desc = "no descriptor";

	/* init in APNetworkInitIfname() */
	ap_ethname = "br-lan";
	//gIfWlanName = NULL;
	wlflag = true;

	/* IP, MAC and default gateway addr will be automatically obtained soon by init_local_addr() */
	ap_ip = 0;
	for(i = 0; i < 6; i++) {
		ap_mac[i] = 0;
	}
	ap_default_gw = 0;

	ap_discovery_type = DISCOVERY_TPYE_DISCOVERY; //broadcast discovery
	static_controller_ip = 0; //static Controller IP addr
	ap_register_service = REGISTERED_SERVICE_CONF_STA; //configuration and station service

	controller_name = NULL;
	controller_desc = NULL;
	controller_ip = 0; 
	for(i = 0; i < 6; i++) {
		controller_mac[i] = 0;
	}
}