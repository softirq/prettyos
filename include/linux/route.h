#ifndef     _ROUTE_H_
#define     _ROUTE_H_

#include "socket.h"

#define  	RTF_UP 			0x0001
#define 	RTF_GATEWAY 	0x0002
#define 	RTF_HOST 		0x0004
#define 	RTF_MSS 		0x0040
#define 	RTF_WINDOW 		0x0080

struct rtentry
{
    unsigned long 	rt_hash;
    struct sockaddr	rt_dst;
    struct sockaddr rt_gateway;
    short 			rt_flags;
    short 			rt_refcnt;
    unsigned long 	rt_use;
    struct ifnet 	*rt_ifp;
    short 			rt_metric;
    char 			*rt_dev;
    unsigned long 	rt_mss;
    unsigned long 	rt_window;
};

#endif
