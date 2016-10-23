#include "error.h"

APErrorHandlingInfo *gLastErrorDataPtr;

APBool APErrorRaise(APErrorCode code, const char *msg) 
{
	if(gLastErrorDataPtr == NULL)
    {
        AP_CREATE_OBJECT_ERR(gLastErrorDataPtr, APErrorHandlingInfo, exit(1););
    }
	
	gLastErrorDataPtr->code = code;
	if(msg != NULL) strcpy(gLastErrorDataPtr->message, msg);
	else gLastErrorDataPtr->message[0]='\0';
	
	return AP_FALSE;
}

APErrorCode APGetLastErrorCode() 
{
    return gLastErrorDataPtr == NULL ? AP_ERROR_GENERAL : gLastErrorDataPtr->code;
}
