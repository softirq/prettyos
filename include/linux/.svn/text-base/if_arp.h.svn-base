
#ifndef _SOCKET_H 
#define _SOCKET_H
#include "socket.h"
#endif

#define 	ATF_COM 	0X02  	//completed entry
#define  	ATF_PERM 	0x04  	//permanent entry
#define  	ATF_PUBL    0x08 	//arp proxy

struct arphdr 
{
		unsigned short 	ar_hwt; 		//type of hardware address (=1, inet)
		unsigned short 	ar_prot; 		//type of protocol (0x800,ip protocol)
		unsigned char 	ar_hlen; 		//length of hardware address
		unsigned char   ar_plen; 		//length of protocol address
		unsigned char   ar_op;  		//arp command type(=1 ARP request,=2 ARP response, =3 RARP request,=4 RARP response)

#if 0
		unsigned char 	ar_sha[ETH_ALEN];
		unsigned char 	ar_sip[4];
		unsigned char 	ar_tha[ETH_ALEN];
		unsigned char  	ar_tip[4];
#endif
};


#define  	ARPOP_REQUEST 		0x01 		//arp request
#define  	ARPOP_REPLY 		0x02  		//arp reply
#define  	RAPROP_REQUEST 		0x03 		//rarp request
#define  	RAPROP_REPLY 		0x03  		//rarp reply


#define 	ARPHRD_ETHER  		0x01
#define  	ARPHRD_AX25 		0x03
#define  	ARPHRD_IEEE802      0x06

struct arpreq
{
		struct sockaddr arp_pa; //protocol address
		struct sockaddr arp_ha; //hardware address
		struct sockaddr arp_netmask;
		int 			arp_flags;
};
