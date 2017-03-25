#include "common.h"

int APTimevalSubtract(struct timeval *res, const struct timeval *x, const struct timeval *y)
{
	int nsec;
	struct timeval z=*y;
   
	if (x->tv_usec < z.tv_usec) {
		nsec = (z.tv_usec - x->tv_usec) / 1000000 + 1;
		z.tv_usec -= 1000000 * nsec;
		z.tv_sec += nsec;
	}
	if (x->tv_usec - z.tv_usec > 1000000) {
		nsec = (x->tv_usec - z.tv_usec) / 1000000;
		z.tv_usec += 1000000 * nsec;
		z.tv_sec -= nsec;
	}

	// compute the time remaining to wait. `tv_usec' is certainly positive
	if ( res != NULL){
		res->tv_sec = x->tv_sec - z.tv_sec;
		res->tv_usec = x->tv_usec - z.tv_usec;
	}

	// return 1 if result is negative (x < y)
	return ((x->tv_sec < z.tv_sec) || ((x->tv_sec == z.tv_sec) && (x->tv_usec < z.tv_usec)));
}

u8 charToHex(char c)
{
    switch(c) 
	{  
		case '0'...'9':  
			return (u8)atoi(&c);  
		case 'a'...'f':  
			return 0xa + (c-'a');  
		case 'A'...'F':  
			return 0xa + (c-'A');  
		default:  
			return 0;
    }
}

void APMACStringToHex(char *str, int* hex) 
{
	int i;
	for(i = 0; i < 6; i++) {
		hex[i] = (charToHex(str[i*3]) << 4) + charToHex(str[i*3 + 1]);
	}
}