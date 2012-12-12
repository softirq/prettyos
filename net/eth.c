#include "type.h"
#include "const.h"
#include "traps.h"
//#include "string.h"
#include "sched.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "kernel.h"
#include "lib.h"
#include "asm-i386/byteorder.h"

#include "linux/netdevice.h"
#include "linux/if_ether.h"
#include "arp.h"

int eth_header(unsigned char *buff,struct net_device *dev,unsigned short type,void *daddr, void *saddr,unsigned int len,struct sk_buff *skb)
{

	struct ethhdr *eth = (struct ethhdr *)buff;
	eth->h_proto = htons(len);

	if(saddr)
	{
		memncpy(eth->h_source,saddr,dev->addr_len);
	}
	if(daddr)
	{
		memncpy(eth->h_dest,daddr,dev->addr_len);
		return dev->hard_header_len;
	}

	return -dev->hard_header_len;

	return 0;
}

int eth_rebuild_header(void *buff,struct net_device *dev,unsigned long dst,struct sk_buff *skb)
{
	struct ethhdr *eth = (struct ethhdr *)buff;
	if(eth->h_proto != htons(ETH_P_IP))
	{
		printk("eth_rebuild_header : dont't know how to resolve type %d address?\n",(int)eth->h_proto);
		memncpy(eth->h_source,dev->dev_addr,dev->addr_len);
		return 0;
	}


	return arp_find(eth->h_dest,dst,dev,dev->pa_addr,skb)?1:0;

	return 0;
}

unsigned int eth_type_trans(struct sk_buff *skb,struct net_device *dev)
{
	struct ethhdr *eth = (struct ethhdr*)skb->data;

	if(*(eth->h_dest) & 0x01)
	{
		if(memcmp(eth->h_dest,dev->broadcast,ETH_ALEN) == 0)
			skb->pkt_type = PACKET_BROADCAST;
		else 
			skb->pkt_type = PACKET_MULTICAST;
	}
	return eth->h_proto;
}

