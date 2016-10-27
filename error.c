#include "error.h"

APErrorHandlingInfo *gLastErrorDataPtr;

APBool APInitErrorHandling() 
{	
	AP_CREATE_OBJECT_ERR(gLastErrorDataPtr, APErrorHandlingInfo, return AP_FALSE;);
	gLastErrorDataPtr->code = AP_ERROR_NONE;

	return AP_TRUE;
}

APBool APErrorRaise(APErrorCode code, const char *msg) 
{
	gLastErrorDataPtr->code = code;
	if(msg != NULL) strcpy(gLastErrorDataPtr->message, msg);
	else gLastErrorDataPtr->message[0]='\0';
	
	return AP_FALSE;
}

APErrorCode APGetLastErrorCode() 
{
    return gLastErrorDataPtr == NULL ? AP_ERROR_GENERAL : gLastErrorDataPtr->code;
}
