#include "ap.h"

u32 gSeqNum;

u16 gAPID;
u8 gDiscoveryType;
u8 gRegisteredService;

char* gControllerName;
char* gControllerDescriptor;
u32 gControllerIPAddr; 
u8 gControllerMACAddr[6];

char* gAPName;
char* gAPDescriptor;
u32 gAPIPAddr;
u8 gAPMACAddr[6];
u32 gAPDefaultGateway;

char* gSSID;
u8 gChannel;
u8 gHardwareMode;
u8 gSuppressSSID;
u8 gSecuritySetting;

#define AP_SETTINGS_FILE 	"setting.conf"
FILE* gSettingsFile = NULL;

__inline__ u32 APGetSeqNum() { return gSeqNum; }

__inline__ u16 APGetAPID() { return gAPID; }
__inline__ u8 APGetDiscoveryType() { return gDiscoveryType; }
__inline__ u8 APGetRegisteredService() { return gRegisteredService; }

__inline__ char* APGetControllerName() { return gControllerName; }
__inline__ char* APGetControllerDescriptor() { return gControllerDescriptor; }
__inline__ u32 APGetControllerIPAddr() { return gControllerIPAddr; }
__inline__ u8* APGetControllerMACAddr() { return gControllerMACAddr; }

__inline__ char* APGetAPName() { return gAPName; }
__inline__ char* APGetAPDescriptor() { return gAPDescriptor; }
__inline__ u32 APGetAPIPAddr() { return gAPIPAddr; }
__inline__ u8* APGetAPMACAddr() { return gAPMACAddr; }
__inline__ u32 APGetAPDefaultGateway() { return gAPDefaultGateway; }

__inline__ char* APGetSSID() { return gSSID; }
__inline__ u8 APGetChannel() { return gChannel; }
__inline__ u8 APGetHardwareMode() { return gHardwareMode; }
__inline__ u8 APGetSuppressSSID() { return gSuppressSSID; }
__inline__ u8 APGetSecuritySetting() { return gSecuritySetting; }

/**
 * get one "useful" (not a comment, not blank) line from the config file
 * @param	configFile [config file]
 * @return	[a "useful" line]
 */
char * APGetLine(FILE *configFile) {

	char *buff = NULL;
	char *command = NULL;
	char *ret = NULL;

	AP_CREATE_STRING_ERR(buff, AP_BUFFER_SIZE, return NULL;);
	
	while ( ((ret = fgets(buff, AP_BUFFER_SIZE, configFile)) != NULL) &&\
		(buff[0] == '\n' || buff[0] == '\r' || buff[0] == '#') );
	
	if(buff != NULL && ret != NULL) {

		int len = strlen(buff);
		buff[len-1] = '\0';
		
		AP_CREATE_STRING_ERR(command, len-1, return NULL;);
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
 * init ap, including seqnum, apid, etc
 */
void APInit()
{
	srand((int)time(NULL));

	gSeqNum = rand();
	gAPID = 0;
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

		if (!strcmp(APExtractTag(line), "AP_NAME"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			AP_CREATE_STRING_ERR(gAPName, len, return AP_FALSE;);
			AP_ZERO_MEMORY(gAPName, len + 1);
			AP_COPY_MEMORY(gAPName, value, len);
			AP_FREE_OBJECT(line);
			continue;	
		}

		if (!strcmp(APExtractTag(line), "AP_DESCRIPTOR"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			AP_CREATE_STRING_ERR(gAPDescriptor, len, return AP_FALSE;);
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
		if (!strcmp(APExtractTag(line), "SSID"))
		{
			char* value = APExtractStringVaule(pos+1);
			int len = strlen(value);
			
			AP_CREATE_STRING_ERR(gSSID, len, return AP_FALSE;);
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
		if (!strcmp(APExtractTag(line), "AUTH_ALGS"))
		{
			gSecuritySetting = APExtractIntVaule(pos+1);
			
			AP_FREE_OBJECT(line);
			continue;	
		}
	}
	return AP_TRUE;
}