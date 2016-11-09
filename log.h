#ifndef __LOG_H__
#define __LOG_H__

#include "common.h"

extern char *gAPLogFileName;
extern int gAPLogLevel;
extern int gAPStdoutLevel;

void APInitLogFile();
void APCloseLogFile();
extern void APLog(const char *format, ...);
extern void APErrorLog(const char *format, ...);
extern void APDebugLog(int level, const char *format, ...);

#endif // LOG_H