#ifndef _SOCKET_H 
#define _SOCKET_H
#include "socket.h"
#endif
/*
* standard interface flags
**/
#define  	IP_ALEN 			4

// interface flags
#define 	IFF_DOWN 		0x0  	//interface is down
#define 	IFF_UP      	0x1  	//interface is up
#define 	IFF_BROADCAST   0x2  	//broadcast address valid
#define 	IFF_LOOPBACK 	0x08 	//is a loopback net 
#define 	IFF_RUNNING 	0x40 	//resource allocated
#define 	IFF_NOARP 		0x80 	//no arp protocol

/*
 * net_device mapping structure
 */
struct ifmap
{
		unsigned long mem_start;  	//硬件读写缓冲区首地址
		unsigned long mem_end;     	
		unsigned short base_addr;   	//I/O端口地址
		unsigned char irq;
		unsigned char dma; 				//DMA通信号
//		unsigned char port; 	
};


/*
 * information of a interface
 */
struct ifreq
{
#define  	IFNAMSIZ 		16
#define 	IFHWADDRLEN  	6	
		union
		{
				char ifrn_name[IFNAMSIZ];
				char ifrn_hwaddr[IFHWADDRLEN];
		}ifr_ifrn;

		union
		{
				struct sockaddr ifru_addr;
				struct sockaddr ifru_broadaddr;
				struct sockaddr ifru_netmask;
				struct sockaddr ifru_hwadddr;
				short 	ifru_flags;
				int 	ifru_metric;
				int 	ifru_mtu;
				struct 	ifmap ifru_map;
				char 	*ifru_data;
		}ifr_ifru;
};

#define  	ifr_name 		ifr_ifrn.ifrn_name
#define  	ifr_hwaddr 		ifr_ifrn.firn_hwaddr
#define 	ifr_addr 		ifr_ifru.ifru_addr 
#define  	ifr_broadcast 	ifr_ifru.ifru_broadaddr
#define  	ifr_netmask 	ifr_ifru.ifru_netmask
#define 	ifr_mtu 		ifr_ifru.ifru_mtu

struct ifconf
{
		int ifc_len;
		union
		{
				char *ifcu_buf;
				struct ifreq *ifcu_req;
		}ifc_ifcu;
};

#define 	ifc_buf 		ifc_ifcu.ifcu_buf
#define 	ifc_req 		ifc_ifcu.ifcu_req

