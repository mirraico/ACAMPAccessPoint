#ifndef __LOG_H__
#define __LOG_H__

#include "common.h"

extern char *gLogFileName;
extern FILE *gLogFile;
extern int gLogLevel;
extern int gStdoutLevel;

void APInitLogFile();
void APCloseLogFile();
extern void APLog(const char *format, ...);
extern void APDebugLog(int level, const char *format, ...);

#endif // LOG_H