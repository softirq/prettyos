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
#include "mm.h"
#include "linux/timer.h"
#include "errno.h"
#include "stddef.h"
#include "asm-i386/byteorder.h"

#include "asm-i386/bitops.h"
#include "asm-i386/system.h"

#include "stddef.h"
#ifndef _NOTIFIER_H
#define _NOTIFIER_H
#include "linux/notifier.h"
#endif
#include "linux/netdevice.h"
#include "linux/sockios.h"
#include "linux/if.h"
#include "linux/in.h"
#include "arp.h"


struct notifier_block *netdev_chain = NULL;
struct packet_type *ptype_base = NULL;

/*
 * backlog is the all packet through the buffer queue from driver layer to link layer
 */
static struct sk_buff_head backlog =
{
		(struct sk_buff *)&backlog,
		(struct sk_buff *)&backlog
};

// backlog queue size
static int backlog_size = 0;

//the 混杂模式's count of the ptype_base queue 
//static int dev_nit = 0;

void dev_add_pack(struct packet_type *pt)
{
		pt->next = ptype_base;
		ptype_base = pt;
}

void dev_remove_pack(struct packet_type *packet_t)
{
		struct packet_type **pt;
		for(pt = &ptype_base;(*pt) != NULL;pt = &((*pt)->next))
		{
				if(*pt == packet_t)
				{
						*pt = packet_t->next;
						return;
				}
		}
}

/*
 * get an net_device  by name
 */

struct net_device *dev_get(char *name)
{
		struct net_device *dev;
		for(dev = dev_base; dev != NULL; dev++)
		{
				if((strncmp(dev->name,name,strlen(dev->name))) == 0)
				{
						return dev;
				}
		}
		return NULL;
}

int dev_open(struct net_device *dev)
{
		int ret = 0;
		if(dev->open)
				ret = dev->open(dev);
		if(ret == 0)
		{
				dev->flags |= IFF_UP | IFF_RUNNING;
				notifier_call_chain((struct notifier_block **)(&netdev_chain),NETDEV_UP,dev);
		}
		return ret;
}

int dev_close(struct net_device *dev)
{
		int ct = 0;
		if(dev->flags != IFF_DOWN)
		{
				if(dev->stop)
						dev->stop(dev);
				notifier_call_chain((struct notifier_block **)(&netdev_chain),NETDEV_DOWN,dev);
				//ip_rt_flush(dev);
				arp_device_down(dev);
				dev->pa_addr = 0;
				dev->pa_broadcast = 0;
				dev->pa_mask = 0;
				while(ct < DEV_NUMBUFFS)
				{
						struct sk_buff *skb;
						while((skb = skb_dequeue(&dev->buffs[ct])) != NULL)
						{
								if(skb->free)
			//							skb_free(skb,FREE_WRITE);
										ct++;
						}
				}
		}
		return 0;
		
				return 0;
}

int register_netdevice_notifier(struct notifier_block *nb)
{
		return notifier_chain_register(&netdev_chain,nb);
}

int unregister_netdevice_notifier(struct notifier_block *nb)
{
		return notifier_chain_unregister(&netdev_chain,nb);
}

//send packet
void dev_queue_xmit(struct sk_buff *skb, struct net_device *dev, int pri) 
{
//		unsigned long flags;
//		struct packet_type *ptype;
		int where = 0;

		if(dev == NULL)
		{
				printk("function dev_queue_xmit : dev is NULL\n");
				return;
		}

		if(skb->next != NULL)
		{
				printk("dev_queue_xmit : worked around a missed interrupt:\n");
				dev->hard_start_xmit(NULL,dev);
				return;
		}

		if(pri < 0 )
		{
				pri = -pri -1;
				where = 1;
		}
		if(pri > DEV_NUMBUFFS)
		{
				printk("bad priority in dev_queue_xmit\n");
				pri = 1;
		}
	//	haven't parse mac address 
		if(!skb->arp && dev->rebuild_header(skb->data,dev,skb->raddr,skb))
		{
				return;
		}

		//packet  from ip or arp
		if(!where)
		{
				skb_queue_tail(dev->buffs + pri,skb);
				skb = skb_dequeue(dev->buffs + pri);
		}

		//driver send packet 
		if(dev->hard_start_xmit(skb,dev) == 0)
				return;
		// send error. insert into the head of queue
		else
		{
				skb_queue_head(dev->buffs + pri, skb);
				return;
		}

}

//receive packet

void netif_rx(struct sk_buff *skb)
{
		static int dropping = 0;
		skb->sk = NULL;
		skb->free = 1;

		if(!backlog_size)
				dropping = 0;
		else if(!backlog_size > 300)
				dropping = 1;

		if(dropping)
		{
//				kfree_skb(skb,FREE_READ);
				return;
		}
		skb_queue_tail(&backlog,skb);
		backlog_size++;

	//	mark_bh(NET_BH);
		return;
}

/*
 * this route is called when an net_device driver is ready to transmit a packet
 */
void dev_tint(struct net_device *dev)
{
		int i;
		struct sk_buff *skb;

		for(i = 0;i < DEV_NUMBUFFS; i++)
		{
				while((skb = skb_dequeue(&dev->buffs[i])) != NULL)
				{
						dev_queue_xmit(skb,dev,-i-1);
						if(dev->tbusy)
								return;
				}
		}
		return;
}

/*
 * traversal the net_device and send some data
 */
void dev_transmit(void)
{
		struct net_device *dev;
		for(dev = dev_base;dev != NULL; dev = dev->next)
		{
				if(dev->flags != 0 && dev->tbusy)
						dev_tint(dev);
		}
}

volatile char in_bh = 0;

int in_net_bh()
{
		return(in_bh == 0?0:1);
}

void net_bh(void *tmp)
{

		struct sk_buff *skb;
		struct packet_type *pt_prev,*ptype;
		unsigned short type;

		if(set_bit(1,(void *)&in_bh))
						return;
		dev_transmit();
		cli();

		while((skb = skb_dequeue(&backlog)) != NULL)
		{
				backlog_size --;
				sti();

				skb->h.raw = skb->data + skb->dev->hard_header_len;
				skb->len = skb->dev->hard_header_len;

				type = skb->dev->type_trans(skb,skb->dev);

				pt_prev = NULL;

				for(ptype = ptype_base; ptype != NULL;ptype = ptype->next)
				{
						if(ptype->type == type && (!ptype->dev || ptype->dev == skb->dev))
						{
								if(pt_prev)
								{

										pt_prev->func(skb,skb->dev,pt_prev);
								}
								pt_prev = ptype;
						}	

				}
				if(pt_prev)
						pt_prev->func(skb,skb->dev,pt_prev);

				dev_transmit();
				cli();
		}
		in_bh = 0;
		sti();
		dev_transmit();
}

/*
 * check bitmask for the ioctl calls for devices
 */
static inline int bad_mask(unsigned long mask, unsigned long addr)
{
		if(addr & (mask = ~mask))
				return 1;
		mask = ntohl(mask);
		if(mask & (mask + 1 ))
				return 1;
		return 0;
}

/*
 * get information from interface
 */
static int dev_ifconf(char *arg)
{
		struct ifconf ifc;
		struct ifreq ifr;
		struct net_device *dev;
		char *pos;
		int len;
//		int error;

		memncpy((char *)&ifc,arg,sizeof(struct ifconf));
		len = ifc.ifc_len;
		pos = ifc.ifc_buf;

		for(dev = dev_base; dev != NULL;dev = dev->next)
		{
				if(!(dev->flags & IFF_UP)) 
						continue;
				memset((char *)&ifr,0,sizeof(struct ifreq));
				strcpy(ifr.ifr_name,dev->name);
				(*((struct sockaddr_in *)&ifr.ifr_addr)).sin_addr.s_addr = dev->pa_addr;
				(*((struct sockaddr_in *)&ifr.ifr_addr)).sin_family = dev->family;
				memncpy(pos,(char *)&ifr,sizeof(struct ifreq));

				pos += sizeof(struct ifreq);
				len -= sizeof(struct ifreq);

				if(len < sizeof(struct ifreq))
						break;
		}

		ifc.ifc_len = (pos - ifc.ifc_buf);
		ifc.ifc_req = (struct ifreq*)ifc.ifc_buf;
		memncpy(arg,(char *)&ifc,sizeof(struct ifconf));

		return (pos - arg);
}

int dev_ioctl(unsigned int cmd, void *arg)
{
		switch(cmd)
		{
				case SIOCGIFFLAGS:
				case SIOCGIFADDR:
						return 0;
				default:
						return 0;

		}
		return -EINVAL;
}

void dev_init()
{
		struct net_device *dev;
		struct net_device *dev_prev = NULL;

		for(dev = dev_base;dev != NULL;dev++)
		{   
				// init failed , delete this net_device from dev list
				if(dev->init &&(dev->init(dev)))
				{   
						if(dev_prev == NULL)
								dev_base = dev->next;
						else
								dev_prev = dev->next;
				}   
				else
						dev_prev = dev;
		}   
		return;
}
