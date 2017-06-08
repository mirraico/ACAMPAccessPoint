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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <string.h>
#include <net/if.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <uci.h>
#include <libubox/uloop.h>
#include "wlconf.h"

#define	BUFFER_SIZE					    65536

#define	zero_memory					    bzero
#define	copy_memory(dst, src, len)	    bcopy(src, dst, len)
#define	create_string(str_name, str_size, on_err)	{str_name = ((char*)malloc(str_size)); if(!(str_name)) {on_err}}
#define	create_object(obj_name, obj_size, on_err)	{obj_name = ((u8*)malloc(obj_size)); if(!(obj_name)) {on_err}}
#define	create_array(ar_name, ar_size, ar_type, on_err)	{ar_name = (ar_type*) (malloc(sizeof(ar_type) * (ar_size))); if(!(ar_name)) {on_err}}
#define	free_object(obj_name)					{if(obj_name){free((obj_name)); (obj_name) = NULL;}}

#define	random_int(min, max) { (min) + (rand() % ((max)-(min))); }


int timeval_subtract(struct timeval *res, const struct timeval *x, const struct timeval *y);
void mac_to_hex(char *str, int* hex);

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;


#endif // COMMON_H
