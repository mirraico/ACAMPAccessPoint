#ifndef __ERROR_H__
#define __ERROR_H__

#include "common.h"

typedef enum {
	AP_ERROR_SUCCESS = 1,
	AP_ERROR_NONE,
	
	AP_ERROR_NOOUTPUT,
	AP_ERROR_BUTNORAISE,

	AP_ERROR_GENERAL,
	AP_ERROR_WARNING,

	AP_ERROR_INTERRUPTED,
	AP_ERROR_OPERATION_ABORTED,
	AP_ERROR_COMUNICATING,
	AP_ERROR_TIME_EXPIRED,

	AP_ERROR_OUT_OF_MEMORY,
	AP_ERROR_WRONG_ARG,
	AP_ERROR_SENDING,
	AP_ERROR_RECEIVING,
	AP_ERROR_INVALID_FORMAT
} APErrorCode;

typedef struct {
	APErrorCode code;
	char message[256];
} APErrorHandlingInfo;

#define APErr(arg)				((arg) || APHandleLastError())

APBool APInitErrorHandling();
APBool APErrorRaise(APErrorCode code, const char *msg);
APErrorCode APGetLastErrorCode();
APBool APHandleLastError();


#endif // ERROR_H