#ifndef     _IP_H_
#define     _IP_H_

#include "type.h"
struct timestamp
{
		u8 len;
		u8 ptr;
};

#define  	MAX_ROUTE 	16

struct route
{
		char route_size;
		char pointer;
		unsigned long route[MAX_ROUTE];
};

struct options
{
		struct route 		record_route;
		struct route 		loose_route;
		struct route 		strict_route;
		struct timestamp 	tstamp;
		unsigned short    	security;                                                      
		unsigned short   	compartment;                                                   
		unsigned short    	handling;                                                      
		unsigned short    	stream;                                                        
		unsigned    		tcc; 
};

struct iphdr 
{
		u8 		version:4,
				ihl:4;
		u8 		tos;
		u16 	tot_len;
		u16 	id;
		u16 	flags_off;
		u8 		ttl;
		u8 		protocol;
		u16 	chksum;
		u32 	saddr;
		u32 	daddr;
};

#endif
