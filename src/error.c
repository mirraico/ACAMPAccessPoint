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
	if(code == AP_ERROR_BUTNORAISE) return AP_FALSE; //don't cover the last error code

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
			
		case AP_ERROR_GENERAL:
			APErrorLog("Error Occurred in %s", infoPtr->message);
			break;

		case AP_ERROR_WARNING:
			APErrorLog("Warning Occurred in %s", infoPtr->message);
			break;
			
		case AP_ERROR_OUT_OF_MEMORY:
			APErrorLog("Out of Memory in %s", infoPtr->message);
			// exit(1);
			break;
			
		case AP_ERROR_WRONG_ARG:
			APErrorLog("Wrong Arguments in %s", infoPtr->message);
			break;
			
		case AP_ERROR_SENDING:
			APErrorLog("Error Sending in %s", infoPtr->message);
			break;
		
		case AP_ERROR_RECEIVING:
			APErrorLog("Error Receiving in %s", infoPtr->message);
			break;
			
		case AP_ERROR_INVALID_FORMAT:
			APErrorLog("Invalid Format in %s", infoPtr->message);
			break;
				
		case AP_ERROR_NOOUTPUT:
		default:
			break;
	}
	
	return AP_FALSE;
}