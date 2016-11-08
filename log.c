#include "log.h"

char *gLogFileName;
FILE *gLogFile = NULL;
int gLogLevel;
int gStdoutLevel;

void APInitLogFile()
{
	if(gLogFileName == NULL) 
    {
		APErrorLog("Wrong File Name for Log File");
	}
	if((gLogFile = fopen(gLogFileName, "w")) == NULL) 
    {
		APErrorLog("Can't open log file: %s", strerror(errno));
		exit(1);
	}
}

void APCloseLogFile()
{
	fclose(gLogFile);
}

__inline__ void _APDebugLog(int level, const char *format, va_list args) 
{
    char *logStr = NULL;
    time_t curtime;
    char *timestr = NULL;
    char label[10];
    
    if(gLogLevel < level && gStdoutLevel < level) return;
    if(format == NULL) return;
    
    time(&curtime);
    timestr = ctime(&curtime);
    
    timestr[strlen(timestr)-1] = '\0';
    
    AP_CREATE_STRING_SIZE_ERR(logStr, (strlen(format)+strlen(timestr)+100), return;);
    
    AP_ZERO_MEMORY(label, 10);
    if(level == -1) strcpy(label, "ERROR ");
    else if(level == 0) strcpy(label, "LOG   ");
    else sprintf(label, "DEBUG%d", level);
    sprintf(logStr, "[AP-%s::%s]\t %s\n", label, timestr, format);
    
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
    
    AP_FREE_OBJECT(logStr);
}

__inline__ void APLog(const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	_APDebugLog(0, format, args);
	va_end(args);
}

__inline__ void APErrorLog(const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	_APDebugLog(-1, format, args);
	va_end(args);
}

__inline__ void APDebugLog(int level, const char *format, ...) 
{
    va_list args;
    va_start(args, format);
    _APDebugLog(level, format, args);
    va_end(args);
}