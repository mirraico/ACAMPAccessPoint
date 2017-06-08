#ifndef __SETTING_H__
#define __SETTING_H__

#include "common.h"
#include "ap.h"
#include "protocol.h"
#include "log.h"

#define SETTINGS_FILE 	"setting.conf"

bool parse_settings_file();
void init_default_settings();


#endif // SETTING_H