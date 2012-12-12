#include "stddef.h"
#include "linux/netdevice.h"

int loopback_init(struct net_device *dev)
{
		dev->mtu = 2000;
		return 0;
}
