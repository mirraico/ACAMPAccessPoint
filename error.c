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

APBool APHandleLastError()
{
	APErrorHandlingInfo *infoPtr = gLastErrorDataPtr;
	
	if(infoPtr == NULL || infoPtr->code == AP_ERROR_NONE) {
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
			APErrorLog("Out of Memory");
			exit(1);
			break;
			
		case AP_ERROR_WRONG_ARG:
			APErrorLog("Wrong Arguments in Function");
			break;
			
		case AP_ERROR_NEED_RESOURCE:
			APErrorLog("Missing Resource");
			break;
			
		case AP_ERROR_GENERAL:
			APErrorLog("Error Occurred");
			break;
		
		case AP_ERROR_CREATING:
			APErrorLog("Error Creating Resource");
			break;
			
		case AP_ERROR_SENDING:
			APErrorLog("Error Sending");
			break;
		
		case AP_ERROR_RECEIVING:
			APErrorLog("Error Receiving");
			break;
			
		case AP_ERROR_INVALID_FORMAT:
			APErrorLog("Invalid Format");
			break;
				
		default:
			break;
	}
	
	return AP_FALSE;
}