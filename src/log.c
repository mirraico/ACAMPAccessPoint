#include "log.h"

char *log_filename;
int log_level;
int log_stdlev;

static FILE *log_file = NULL;

void init_log()
{
	if(log_filename == NULL) 
	{
		log_file = NULL;
		log_i("The log_i File is disabled");
		return;
	}
	if((log_file = fopen(log_filename, "w")) == NULL) 
	{
		log_e("Can't open log file: %s", strerror(errno));
		exit(1);
	}
}

void destroy_log()
{
	fclose(log_file);
	log_file = NULL;
}

static __inline__ void _log_d(int level, const char *format, va_list args) 
{
	char *logstr = NULL;
	time_t curtime;
	char *timestr = NULL;
	char label[10];
	
	if(log_level < level && log_stdlev < level) return;
	if(format == NULL) return;
	
	time(&curtime);
	timestr = ctime(&curtime);
	
	timestr[strlen(timestr)-1] = '\0';
	
	create_string(logstr, (strlen(format)+strlen(timestr)+100), return;);
	
	zero_memory(label, 10);
	if(level == -1) strcpy(label, "ERROR ");
	else if(level == 0) strcpy(label, "LOG   ");
	else sprintf(label, "DEBUG%d", level);
	sprintf(logstr, "[AP-%s::%s]\t %s\n", label, timestr, format);
	
	char fileline[256];
	vsnprintf(fileline, 255, logstr, args);

	if(log_level >= level && log_file != NULL) 
	{
		fwrite(fileline, strlen(fileline), 1, log_file);
		fflush(log_file);
	}
	if(log_stdlev >= level)
	{
		printf("%s", fileline);
	}
	
	free_object(logstr);
}

__inline__ void log_i(const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	_log_d(0, format, args);
	va_end(args);
}

__inline__ void log_e(const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	_log_d(-1, format, args);
	va_end(args);
}

__inline__ void log_d(int level, const char *format, ...) 
{
	va_list args;
	va_start(args, format);
	_log_d(level, format, args);
	va_end(args);
}