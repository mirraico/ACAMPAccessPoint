#ifndef __SETTING_H__
#define __SETTING_H__

#include "common.h"
#include "ap.h"
#include "log.h"

#define AP_SETTINGS_FILE 	"setting.conf"

extern FILE* gSettingsFile;

APBool APParseSettingsFile();
void APDefaultSettings();
// APBool APCheckSettings();


#endif // SETTING_H