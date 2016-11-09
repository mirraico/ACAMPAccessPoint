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
		if (!strcmp(APExtractTag(line), "HD_SYS_LOG_MODULES"))
		{
			gHdSysLogModules = APExtractIntVaule(pos+1);
			
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "HD_SYS_LOG_LEVEL"))
		{
			gHdSysLogLevel = APExtractIntVaule(pos+1);
			
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "HD_STDOUT_LOG_MODULES"))
		{
			gHdStdoutModules = APExtractIntVaule(pos+1);
			
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "HD_STDOUT_LOG_LEVEL"))
		{
			gHdStdoutLevel = APExtractIntVaule(pos+1);
			
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
			AP_FREE_OBJECT(line);
			continue;
		}
		if (!strcmp(APExtractTag(line), "DIDCOVERY_TYPE"))
		{
			gDiscoveryType = APExtractIntVaule(pos+1);
			
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
			AP_FREE_OBJECT(line);
			AP_FREE_OBJECT(ip);
			continue;
		}
		if (!strcmp(APExtractTag(line), "REGISTERED_SERVICE"))
		{
			gRegisteredService = APExtractIntVaule(pos+1);
			
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "SSID"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			AP_CREATE_STRING_SIZE_ERR(gSSID, len + 1, return AP_FALSE;);
			AP_ZERO_MEMORY(gSSID, len + 1);
			AP_COPY_MEMORY(gSSID, value, len);
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "SUPPRESS_SSID"))
		{
			gSuppressSSID = APExtractIntVaule(pos+1);
			
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "HW_MODE"))
		{
			gHardwareMode = APExtractIntVaule(pos+1);
			
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "CHANNEL"))
		{
			gChannel = APExtractIntVaule(pos+1);
			
			AP_FREE_OBJECT(line);
			continue;	
		}
		if (!strcmp(APExtractTag(line), "SECURITY_OPTION"))
		{
			gSecurityOption = APExtractIntVaule(pos+1);
			
			AP_FREE_OBJECT(line);
			continue;	
		}
	}
	return AP_TRUE;
}


void APDefaultSettings()
{
	int i;

	gAPLogFileName = NULL; //no file log
	gAPLogLevel = 0;
	gAPStdoutLevel = 0;

	gHdSysLogModules = -1;
	gHdSysLogLevel = 4;
	gHdStdoutModules = -1;
	gHdStdoutLevel = 4;

	gAPName = "AP";
	gAPDescriptor = "no descriptor";
	/* IP, MAC and default gateway addr will be automatically obtained soon by APNetworkInitLocalAddr() */
	gAPIPAddr = 0;
	for(i = 0; i < 6; i++) {
		gAPMACAddr[i] = 0;
	}
	gAPDefaultGateway = 0;

    gDiscoveryType = 0; //broadcast discovery
	gRegisteredService = 0; //configuration and station service

	gSSID = "ap_ssid";
	gSuppressSSID = 0;
	gHardwareMode = 2;
	gChannel = 7;
	gSecurityOption = 0;

	gControllerName = NULL;
	gControllerDescriptor = NULL;
	gControllerIPAddr = 0; 
	for(i = 0; i < 6; i++) {
		gControllerMACAddr[i] = 0;
	}
}