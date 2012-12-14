#ifndef     _IN_H_
#define     _IN_H_

enum
{
    IPPROTO_IP = 0,
    IPPROTO_ICMP = 1,
    IPPROTO_IGMP = 2,
    IPPROTO_TCP = 6,
    IPPROTO_UDP = 17,
    IPPROTO_RAW = 255,
    IPPROTO_MAX
};

#define 	INADDR_LOOPBACK 		((unsigned long int)0x7f000001)
#define    	INADDR_ANY 				((unsigned long int)0x00000000)
#define  	INADDR_BROADCAST 		((unsigned long int)0xffffffff)


#define 	INADDR_ANY      		((unsigned long int)0x00000000)   


#define  	IN_IPCLASSA(addr) 		(((unsigned long)addr & 0x80000000) == 0)
#define  	IN_IPCLASSB(addr) 		(((unsigned long)addr & 0x40000000) == 0)
#define  	IN_IPCLASSC(addr) 		(((unsigned long)addr & 0x20000000) == 0)

#define 	IN_IPCLASSA_NET 			0x7f000000  //max of class a
#define 	IN_IPCLASSB_NET 			0xbf000000  // max of class b
#define 	IN_IPCLASSC_NET 			0Xdf000000  // max of class c

#define 	IN_IPCLASSA_NET_MASK 	 	0xff000000L
#define 	IN_IPCLASSB_NET_MASK 	 	0xffff0000L
#define 	IN_IPCLASSC_NET_MASK	 	0xffffff00L

struct in_addr
{
    unsigned long 	s_addr;
};

struct sockaddr_in
{
    unsigned short 	sin_len;
    unsigned short 	sin_family;
    unsigned short 	sin_port;
    struct in_addr 	sin_addr;
    char 	sin_zero[8];
};

#endif
