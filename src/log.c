#include "log.h"

char *gAPLogFileName;
int gAPLogLevel;
int gAPStdoutLevel;

FILE *gAPLogFile = NULL;

void APInitLogFile()
{
	if(gAPLogFileName == NULL) 
	{
		gAPLogFile = NULL;
		APLog("The Log File is disabled");
		return;
	}
	if((gAPLogFile = fopen(gAPLogFileName, "w")) == NULL) 
	{
		APErrorLog("Can't open log file: %s", strerror(errno));
		exit(1);
	}
}

void APCloseLogFile()
{
	fclose(gAPLogFile);
	gAPLogFile = NULL;
}

__inline__ void _APDebugLog(int level, const char *format, va_list args) 
{
	char *logStr = NULL;
	time_t curtime;
	char *timestr = NULL;
	char label[10];
	
	if(gAPLogLevel < level && gAPStdoutLevel < level) return;
	if(format == NULL) return;
	
	time(&curtime);
	timestr = ctime(&curtime);
	
	timestr[strlen(timestr)-1] = '\0';
	
	create_string(logStr, (strlen(format)+strlen(timestr)+100), return;);
	
	zero_memory(label, 10);
	if(level == -1) strcpy(label, "ERROR ");
	else if(level == 0) strcpy(label, "LOG   ");
	else sprintf(label, "DEBUG%d", level);
	sprintf(logStr, "[AP-%s::%s]\t %s\n", label, timestr, format);
	
	char fileLine[256];
	vsnprintf(fileLine, 255, logStr, args);

	if(gAPLogLevel >= level && gAPLogFile != NULL) 
	{
		fwrite(fileLine, strlen(fileLine), 1, gAPLogFile);
		fflush(gAPLogFile);
	}
	if(gAPStdoutLevel >= level)
	{
		printf("%s", fileLine);
	}
	
	free_object(logStr);
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