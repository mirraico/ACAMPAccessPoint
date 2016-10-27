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
	
	/* keep return false */
	return AP_FALSE;
}

APErrorCode APGetLastErrorCode() 
{
    return gLastErrorDataPtr == NULL ? AP_ERROR_NONE : gLastErrorDataPtr->code;
}

APBool APHandleLastError()
{
	APErrorHandlingInfo *infoPtr = gLastErrorDataPtr;
	
	if(infoPtr == NULL) {
		APErrorLog("Encounter fallure but no error pending");
		exit(1);
		return AP_FALSE;
	}
	
	switch(infoPtr->code) {
		case AP_ERROR_SUCCESS:
		case AP_ERROR_NONE:
			return AP_TRUE;
			break;
			
		case AP_ERROR_OUT_OF_MEMORY:
			APErrorLog("Out of Memory %s", infoPtr->message);
			exit(1);
			break;
			
		case AP_ERROR_WRONG_ARG:
			APErrorLog("Wrong Arguments in Function %s", infoPtr->message);
			break;
			
		case AP_ERROR_NEED_RESOURCE:
			APErrorLog("Missing Resource %s", infoPtr->message);
			break;
			
		case AP_ERROR_GENERAL:
			APErrorLog("Error Occurred %s", infoPtr->message);
			break;
		
		case AP_ERROR_CREATING:
			APErrorLog("Error Creating Resource %s", infoPtr->message);
			break;
			
		case AP_ERROR_SENDING:
			APErrorLog("Error Sending %s", infoPtr->message);
			break;
		
		case AP_ERROR_RECEIVING:
			APErrorLog("Error Receiving %s", infoPtr->message);
			break;
			
		case AP_ERROR_INVALID_FORMAT:
			APErrorLog("Invalid Format %s", infoPtr->message);
			break;
				
		default:
			break;
	}
	
	return AP_FALSE;
}