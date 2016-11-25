#include "hostapd.h"
#include "ap.h"
#include "protocol.h"

char *gHdConfigurationName = "./hostapd.tmp";
FILE *gHdConfiguration = NULL;

APBool HdGenerateConfigurationFile()
{
    APDebugLog(3, "Generate Hostapd Configuration File");
    char fileLine[256];
    AP_ZERO_MEMORY(fileLine, 256);
	if(gHdConfigurationName == NULL) 
    {
		APLog("The Hostapd Configuration File Name is null");
        return AP_FALSE;
	}
	if((gHdConfiguration = fopen(gHdConfigurationName, "w")) == NULL) 
    {
		APErrorLog("Can't open hostapd configuration file: %s", strerror(errno));
		return AP_FALSE;
	}

    sprintf(fileLine, "interface=%s\n", gIfWlanName);
    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
    sprintf(fileLine, "logger_syslog=%d\n", gHdSysLogModules);
    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
    sprintf(fileLine, "logger_syslog_level=%d\n", gHdSysLogLevel);
    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
    sprintf(fileLine, "logger_stdout=%d\n", gHdStdoutModules);
    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
    sprintf(fileLine, "logger_stdout_level=%d\n", gHdStdoutLevel);
    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);

    sprintf(fileLine, "ssid=%s\n", gSSID);
    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
    sprintf(fileLine, "channel=%u\n", gChannel);
    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);

    if(gHardwareMode == HWMODE_A) {
        sprintf(fileLine, "hw_mode=a\n");
        fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
    } else if(gHardwareMode == HWMODE_B) {
        sprintf(fileLine, "hw_mode=b\n");
        fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
    } else if(gHardwareMode == HWMODE_G) {
        sprintf(fileLine, "hw_mode=g\n");
        fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
    } else if(gHardwareMode == HWMODE_AD) {
        sprintf(fileLine, "hw_mode=ad\n");
        fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
    }

    if(gSuppressSSID == SUPPRESS_SSID_ENABLED) {
        sprintf(fileLine, "ignore_broadcast_ssid=1\n");
        fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
    }

    if(gSecurityOption == SECURITY_OPEN) {
        sprintf(fileLine, "auth_algs=1\n");
        fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
    } else if(gSecurityOption == SECURITY_WEP) {
        sprintf(fileLine, "auth_algs=3\n");
        fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
        sprintf(fileLine, "wep_default_key=%d\n", gWEP.default_key);
        fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
        if(gWEP.key0 != NULL) {
            switch(gWEP.key0_type) {
                case WEPTYPE_CHAR5:
                case WEPTYPE_CHAR13:
                case WEPTYPE_CHAR16:
                    sprintf(fileLine, "wep_key0=\"%s\"\n", gWEP.key0);
                    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
                    break;
                case WEPTYPE_HEX10:
                case WEPTYPE_HEX26:
                case WEPTYPE_HEX32:
                    sprintf(fileLine, "wep_key0=%s\n", gWEP.key0);
                    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
                    break;
            }
        }
        if(gWEP.key1 != NULL) {
            switch(gWEP.key1_type) {
                case WEPTYPE_CHAR5:
                case WEPTYPE_CHAR13:
                case WEPTYPE_CHAR16:
                    sprintf(fileLine, "wep_key1=\"%s\"\n", gWEP.key1);
                    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
                    break;
                case WEPTYPE_HEX10:
                case WEPTYPE_HEX26:
                case WEPTYPE_HEX32:
                    sprintf(fileLine, "wep_key1=%s\n", gWEP.key1);
                    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
                    break;
            }
        }
        if(gWEP.key2 != NULL) {
            switch(gWEP.key2_type) {
                case WEPTYPE_CHAR5:
                case WEPTYPE_CHAR13:
                case WEPTYPE_CHAR16:
                    sprintf(fileLine, "wep_key2=\"%s\"\n", gWEP.key2);
                    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
                    break;
                case WEPTYPE_HEX10:
                case WEPTYPE_HEX26:
                case WEPTYPE_HEX32:
                    sprintf(fileLine, "wep_key2=%s\n", gWEP.key2);
                    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
                    break;
            }
        }
        if(gWEP.key3 != NULL) {
            switch(gWEP.key3_type) {
                case WEPTYPE_CHAR5:
                case WEPTYPE_CHAR13:
                case WEPTYPE_CHAR16:
                    sprintf(fileLine, "wep_key3=\"%s\"\n", gWEP.key3);
                    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
                    break;
                case WEPTYPE_HEX10:
                case WEPTYPE_HEX26:
                case WEPTYPE_HEX32:
                    sprintf(fileLine, "wep_key3=%s\n", gWEP.key3);
                    fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
                    break;
            }
        }
    } else if(gSecurityOption == SECURITY_WPA) {
        sprintf(fileLine, "wpa=%u\n", gWPA.version);
        fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);

        if(gWPA.pairwire_cipher == WPA_PAIRWIRECIPHER_TKIP) {
            sprintf(fileLine, "wpa_pairwise=TKIP\nrsn_pairwise=TKIP\n");
            fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
        } else if(gWPA.pairwire_cipher == WPA_PAIRWIRECIPHER_CCMP) {
            sprintf(fileLine, "wpa_pairwise=CCMP\nrsn_pairwise=CCMP\n");
            fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
        } else if(gWPA.pairwire_cipher == WPA_PAIRWIRECIPHER_TKIP_CCMP) {
            sprintf(fileLine, "wpa_pairwise=TKIP CCMP\nrsn_pairwise=TKIP CCMP\n");
            fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
        }

        sprintf(fileLine, "wpa_passphrase=\"%s\"\n", gWPA.password);
        fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
        sprintf(fileLine, "wpa_group_rekey=%u\n", gWPA.group_rekey);
        fwrite(fileLine, strlen(fileLine), 1, gHdConfiguration);
    }

    fflush(gHdConfiguration);

    return AP_TRUE;
}