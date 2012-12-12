#include "errno.h"
#include "stddef.h"
#include "linux/netdevice.h"

struct net_device dev_21140=
{
		0,
		0,
		0,
		0,
		"21140",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
};

//net net_device 12240 get packet from hard-queue and resend by call dev_queue_xmit
static int ei_start_xmit(struct sk_buff *skb,struct net_device *dev)
{
	return 0;
}

int ethdev_init(struct net_device *dev)
{
	dev->hard_start_xmit = &ei_start_xmit;
	ether_setup(dev);	
	return 0;
}

int init_module()
{
		if(register_netdev(&dev_21140) != 0)
		{
				return -EIO;
		}
		return 0;
}
