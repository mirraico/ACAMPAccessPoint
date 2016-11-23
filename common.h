#ifndef __COMMON_H__
#define __COMMON_H__

#include "stdio.h"
#include "stdarg.h"
#include "time.h"
#include "ifaddrs.h"
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <string.h>
#include <net/if.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

typedef enum  {
	AP_FALSE = 0,
	AP_TRUE = 1
} APBool;

#define	AP_BUFFER_SIZE					65536
#define	AP_ZERO_MEMORY					bzero
#define	AP_COPY_MEMORY(dst, src, len)	bcopy(src, dst, len)
#define	AP_REPEAT_FOREVER				while(1)

#define	AP_CREATE_STRING_SIZE_ERR(str_name, str_size, on_err)	{str_name = ((char*)malloc(str_size)); if(!(str_name)) {on_err}}
#define	AP_CREATE_OBJECT_SIZE_ERR(obj_name, obj_size, on_err)	{obj_name = ((u8*)malloc(obj_size)); if(!(obj_name)) {on_err}}
#define	AP_CREATE_OBJECT_ERR(obj_name, obj_type, on_err)	{obj_name = (obj_type*) (malloc(sizeof(obj_type))); if(!(obj_name)) {on_err}}
#define	AP_CREATE_ARRAY_ERR(ar_name, ar_size, ar_type, on_err)	{ar_name = (ar_type*) (malloc(sizeof(ar_type) * (ar_size))); if(!(ar_name)) {on_err}}

#define	AP_FREE_OBJECT(obj_name)					{if(obj_name){free((obj_name)); (obj_name) = NULL;}}
#define	AP_FREE_OBJECTS_ARRAY(ar_name, ar_size)		{int _i = 0; for(_i = ((ar_size)-1); _i >= 0; _i--) {if(((ar_name)[_i]) != NULL){ free((ar_name)[_i]);}} free(ar_name); (ar_name) = NULL; }

#define	AP_RANDOM_INT(min, max) { (min) + (rand() % ((max)-(min))); }


int APTimevalSubtract(struct timeval *res, const struct timeval *x, const struct timeval *y);


typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;


#endif // COMMON_H
