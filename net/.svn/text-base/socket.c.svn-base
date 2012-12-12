#include "type.h"
#include "const.h"
#include "traps.h"
#include "string.h"
#include "sched.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "kernel.h"
#include "errno.h"

#include "linux/net.h"
#include "linux/socket.h"
#include "linux/netdevice.h"


static struct proto_ops *pops[NPROTO];


int sock_register(int family,struct proto_ops *ops)
{
	int i;
	for(i = 0;i < NPROTO;i++)
	{
		if(pops[i] != NULL)
			continue;
		pops[i] = ops;
		pops[i]->family = AF_INET;
		return i;
	}
	return -ENOMEM;
}

void proto_init(void)
{
	extern struct net_proto protocols[];
	struct net_proto *proto;
	proto = protocols;
	while(proto->name != NULL)
	{
		if(proto->init_func)
		{
			(*proto->init_func)(proto);
			proto++;
		}
	}
	return;
}

int init_sock (void)
{
	int i;

	printk("Tencent DIS NET 0.10\n");
	
	for(i = 0;i < NPROTO;i++)
		pops[i] = NULL;

	proto_init();
	dev_init();


	return 0;
}
