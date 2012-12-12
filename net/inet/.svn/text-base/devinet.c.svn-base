#include "stddef.h"
#include "asm-i386/byteorder.h"

#include "linux/in.h"
#include "linux/if.h"
#include "linux/netdevice.h"

unsigned long ip_get_mask(unsigned long addr)
{
		unsigned long dst;

		if(addr == 0L)
				return 0L;

		dst = ntohl(addr);
		if(IN_IPCLASSA(dst))
		{
				return IN_IPCLASSA_NET_MASK;
		}
		else if(IN_IPCLASSB(dst))
		{
				return IN_IPCLASSB_NET_MASK;
		}
		else if(IN_IPCLASSC(dst))
		{
				return IN_IPCLASSC_NET_MASK;
		}
		return 0L;

}

int ip_chk_addr(unsigned long addr)
{
		struct net_device *dev;
		unsigned long mask;

		if(addr == INADDR_ANY|| addr == INADDR_BROADCAST || addr == htons(0x7fffffffL))
		{
				return IS_BROADCAST;
		}

		mask = ip_get_mask(addr);

		//loopback address 127.0.0.0 网段
		if((addr & mask) == htonl(0x7f000000))
		{
				return IS_MYADDR;
		}

		for(dev = dev_base; dev != NULL;dev++)
		{
				if(!(dev->flags & IFF_UP))
						continue;
				if(dev->pa_addr == 0)
						return IS_MYADDR;
				if(addr == dev->pa_addr)
						return IS_MYADDR;
				if((dev->flags & IFF_BROADCAST) && addr == dev->pa_broadcast)
						return IS_BROADCAST;
		}

		return 0;
}
/*
 * retrieve our own address (loopback address)
 */
unsigned long ip_my_addr()
{
		struct net_device *dev;
		for(dev = dev_base; dev != NULL;dev++)
		{
				if(dev->flags & IFF_LOOPBACK)
						return dev->pa_addr;
		}
		return 0;

}

struct net_device * ip_dev_check(unsigned long addr)
{
		struct net_device *dev;
		for(dev = dev_base; dev != NULL; dev++)
		{
				if(!(dev->flags & IFF_UP))
						continue;
				if((addr ^ dev->pa_addr) & dev->pa_mask)
						continue;
				return dev;
		}
		return NULL;

}
