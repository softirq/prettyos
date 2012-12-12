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

#include "errno.h"
#include "stddef.h"

#include "linux/netdevice.h"
#include "eth.h"
#include "socket.h"

#define MAX_ETH_CARDS 	16

struct net_device *ethdev_index[MAX_ETH_CARDS];


void ether_setup(struct net_device *dev)
{
	dev->hard_header = &eth_header;
	dev->type_trans = &eth_type_trans;
	dev->rebuild_header = &eth_rebuild_header;
	dev->family = AF_INET;
	return;
}

int register_netdev(struct net_device *dev)
{
		int i;
		struct net_device *d = dev_base;
		
		if(dev && dev->init)
		{
				if(dev->name && ((dev->name[0] = '\0') || (dev->name[0] =' ')))
				{
						for(i = 0;i < MAX_ETH_CARDS;i++)
						{
								if(ethdev_index[i] == NULL)
								{
										printk("loading net_device %s\n",dev->name);
										ethdev_index[i] = dev;
										break;
								}
						}
				}
				if(dev->init(dev) != 0)
				{

						return -EIO;
				}
		}

		if(dev_base)
		{
				while(d->next)
						d = d->next;
				d->next = dev;
		}
		else 
				dev_base = dev;

		dev->next = NULL;
		return 0;

}

void unregister_netdev(struct net_device *dev)
{
		struct net_device *d = dev_base;
		int i;

		printk("unregister netdev : net_device");
		if(dev == NULL)
		{
				printk("was NULL\n");
				return;
		}

		if(dev->start)
				printk("%s busy\n",dev->name);
		else
		{
				while(d && d->next != dev)
						d = d->next;
				if(d && d->next == dev)
				{
						d->next = dev->next;
						printk("%s unlinked \n",dev->name);
				}
				else
				{
						printk("%s not found \n",dev->name);
						return;
				}
				for(i = 0; i < MAX_ETH_CARDS;i++)
				{
						if(ethdev_index[i] == dev)
								ethdev_index[i] = NULL;
						break;
				}
		}
		return;
}
