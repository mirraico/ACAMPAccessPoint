#ifndef __LOG_H__
#define __LOG_H__

#include "common.h"

extern char *log_filename;
extern int log_level;
extern int log_stdlev;

void init_log();
void destroy_log();
extern void log_i(const char *format, ...);
extern void log_e(const char *format, ...);
extern void log_d(int level, const char *format, ...);

#endif // LOG_H