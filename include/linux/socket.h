#ifndef     _SOCKET_H_
#define     _SOCKET_H_

#define 	AF_UNIX 	1
#define 	AF_INET 	2


#define 	SOCK_ARRAY_SIZE 	256

struct sockaddr
{
    unsigned short sa_len;
    unsigned short sa_family;
    char 			sa_data[14];
};


#define 	SOPRI_INTERACTIVE  	0
#define 	SOPRI_NORMAL 		1
#define  	SOPRI_BACKGROUND  	2	

#define 	IPTOS_LOWDELAY 		0x10
#define 	IPTOS_THROUGHPUT 	0x08
#define 	IPTOS_RELIABILITY 	0x04

/* Flags we can use with send/ and recv. */
#define MSG_OOB     1
#define MSG_PEEK 	2
#define MSG_DONTROUTE   4

#endif
