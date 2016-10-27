#include "log.h"

char *gLogFileName;
FILE *gLogFile = NULL;
int gLogLevel;
int gStdoutLevel;

void APInitLogFile()
{
	if(gLogFileName == NULL) {
		APLog("Wrong File Name for Log File");
	}
	if((gLogFile = fopen(gLogFileName, "w")) == NULL) {
		APLog("Can't open log file: %s", strerror(errno));
		exit(1);
	}
}

void APCloseLogFile()
{
	fclose(gLogFile);
}

__inline__ void APLog(const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	APDebugLog(0, format, args);
	va_end(args);
}

__inline__ void APDebugLog(int level, const char *format, ...) 
{
    char *logStr = NULL;
    va_list args;
    time_t curtime;
    char *timestr = NULL;
    
    if(gLogLevel < level && gStdoutLevel < level) return;
    if(format == NULL) return;
    
    time(&curtime);
    timestr = ctime(&curtime);
    
    timestr[strlen(timestr)-1] = '\0';
    
    AP_CREATE_STRING_ERR(logStr, (strlen(format)+strlen(timestr)+100), return;);
    
    sprintf(logStr, "[[ACMAP-AP::%s]]\t\t %s\n", timestr, format);

    va_start(args, format);
    
    char fileLine[256];
    vsnprintf(fileLine, 255, logStr, args);

    if(gLogLevel >= level && gLogFile != NULL) 
    {
        fwrite(fileLine, strlen(fileLine), 1, gLogFile);
        fflush(gLogFile);
    }
    if(gStdoutLevel >= level)
    {
        printf("%s", fileLine);
    }
    
    va_end(args);
    AP_FREE_OBJECT(logStr);
}