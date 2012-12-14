#include "stddef.h"
#include "linux/netdevice.h"

#define NEXT_DEV 	NULL

extern int loopback_init (struct net_device *dev);
extern int ne_probe(struct net_device *dev);

static int ethif_probe(struct net_device *dev)
{
    ne_probe(dev);
    return 0;
}

struct net_device eth0_dev =
{
    0,
    0,
    0,
    0,
    "eth0",
    ethif_probe,
    NULL,
    NULL,
    NULL,
    NULL,
    NEXT_DEV
};

struct net_device loopback_dev = 
{
    0,
    0,   // start
    0,   	//tbusy
    0,   	//mtu
    "lo", 	//name 
    loopback_init, 		//init
    NULL,  			//hard_start_xmit
    NULL,
    NULL,
    NULL,
    &eth0_dev //next
};

struct net_device *dev_base = &loopback_dev;
