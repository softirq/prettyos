#include "type.h"
#include "const.h"
#include "asm-i386/byteorder.h"
//#include "traps.h"
//#include "sched.h"
//#include "tty.h"
//#include "console.h"
//#include "global.h"
//#include "kernel.h"
#include "errno.h"
#include "lib.h"
#include "stddef.h"

#include "asm-i386/system.h"

#include "linux/netdevice.h"
#include "linux/notifier.h"
#include "linux/in.h"
#include "linux/route.h"
#include "linux/sockios.h"
#include "linux/ip.h"

#include "route.h"

static struct rtable *rt_base = NULL;
static struct rtable *rt_loopback = NULL;

static void rt_del(unsigned long dst,char *devname)
{
    unsigned long flags;
    struct rtable *rt,**rtp;
    rtp = &rt_base;

    save_flags(flags);
    cli();
    while((rt = *rtp) != NULL)
    {
        if(rt->rt_dst  != dst || (devname != NULL && strcmp(rt->rt_dev->name,devname) != 0))
        {
            rtp = &((*rtp)->rt_next);
            continue;
        }
        // delete the rtable entry
        *rtp = rt->rt_next;
        //	if(rt_loopback == rt)
        //			rt_loopback = NULL;
        //			kfree_s(rt,sizeof(struct rtable);

    }
    restore_flags(flags);
    return;
}

/*
 * remove all routing table entries for a net_device . This is called when a net_device is downed.
 */
void ip_rt_flush(struct net_device *dev)
{
    unsigned long flags;
    struct rtable *rt, **rtp;
    rtp = &rt_base;

    save_flags(flags);
    cli();
    while((rt = *rtp) != NULL)
    {
        if(rt->rt_dev != dev)
        {
            rtp = &(*rtp)->rt_next;
            continue;
        }
        *rtp = rt->rt_next;
        //				kfree_s(rt,sizeof(struct rtable));

    }
    restore_flags(flags);
    return;
}

static inline unsigned long default_netmask(unsigned long dst)
{
    dst = ntohl(dst);

    if(IN_IPCLASSA(dst))
        return htonl(IN_IPCLASSA_NET);
    else if(IN_IPCLASSB(dst))
        return htonl(IN_IPCLASSB_NET);
    else if(IN_IPCLASSC(dst))
        return htonl(IN_IPCLASSC_NET);
    return 0L;
}

static inline unsigned long guess_mask(unsigned long dst, struct net_device *dev)
{
    unsigned long mask;
    if(!dst)
        return 0;
    mask = default_netmask(dst);
    if((dst ^ dev->pa_addr) & mask)
        return mask;
    return dev->pa_mask;
}
/*
 *find the route entry through out gataway will be reached
 */
static inline struct net_device * get_gw_dev(unsigned long gw)
{
    struct rtable *rt;
    for(rt = rt_base; rt!= NULL; rt= rt->rt_next)
    {
        if((gw & rt->rt_mask) ^ rt->rt_dst)
            continue;
        if(rt->rt_flags & RTF_GATEWAY)
            return NULL;
        return rt->rt_dev;
    }
    return NULL;
}

void ip_rt_add(short flags, unsigned long dst, unsigned long mask, unsigned long gw, struct net_device *dev, \
        unsigned short mtu, unsigned long window)
{
    struct rtable *rt,*r,*prev;
    struct rtable **rtp; 

    unsigned long rtflags;
    // is dst is host address
    if(flags & RTF_HOST)
        mask = 0xffffffff;

    // network address
    else if(!mask)
    {
        //	if(!((dst ^ dev->pa_mask) ^ dev->pa_addr))
        if(!((dst ^ dev->pa_addr ) & dev->pa_mask ))
        {
            mask = dev->pa_mask;
            flags &= ~RTF_GATEWAY;
        }
        else 
            mask = guess_mask(dst,dev);
        dst &= mask;
    }

    if(gw == dev->pa_addr)
    {
        flags &= ~RTF_GATEWAY;
    }

    if(flags & RTF_GATEWAY)
    {
        if(dev != get_gw_dev(gw))
            return;
        flags |= RTF_GATEWAY;
    }
    else
        gw = 0;

    //		rt = (struct rtable *)kmalloc(sizeof(struct rtable),GFP_ATOMIC);
    if(rt == NULL)
        return;
    memset((char *)rt, 0, sizeof(struct rtable));
    rt->rt_flags = flags | RTF_UP;
    rt->rt_dst = dst;
    rt->rt_dev = dev;
    rt->rt_gateway = gw;
    rt->rt_mask = mask;
    //		rt->rt_mss = dev->mtu - HEADER_SIZE;
    rt->rt_window = 0;

    if(rt->rt_flags & RTF_MSS)
        rt->rt_mss = mtu;
    if(rt->rt_flags & RTF_WINDOW)
        rt->rt_window = window;

    save_flags(rtflags);
    cli();

    rtp = &rt_base;
    while((r = *rtp) != NULL)
    {
        if((r->rt_dst != dst) || r->rt_mask != mask)
        {
            rtp = &r->rt_next;
        }
        else
            *rtp = r->rt_next;
        //			kfree_s(r,sizeof(struct rtable));
    }

    rtp = &rt_base;
    while((r = *rtp) != NULL)
    {
        if((r->rt_mask & mask) != mask)
            break;
        rtp = &r->rt_next;
        prev = r;
    }

    rt->rt_next = r;	
    prev->rt_next = rt;
    //		*rtp = rt;
    //		if((rt->rt_dev->flags & IFF_LOOPBACK) && !rt_loopback)
    //				rt_loopback = rt;
    restore_flags(rtflags);
    return;
}

static inline int bad_mask(unsigned long mask, unsigned long addr)
{
    if(addr & (mask = ~mask))
        return 1;
    mask = ntohl(mask);
    if(mask & (mask + 1))
        return 1;
    return 0;
}
/*
 * process a route add request from the user
 */

static int rt_new(struct rtentry *re)
{
    return 0;
}

int rt_get_info (char *buffer, char **start, off_t offset, int length)
{
    return 0;
}

/*
 * route a packet
 */
struct rtable* ip_rt_route(unsigned long daddr, struct options *opt, unsigned long *src_addr)
{
    struct rtable *rt;
    for(rt = rt_base; rt != NULL; rt = rt->rt_next)
    {
        if(!((daddr  & rt->rt_mask) ^ rt->rt_dst))
            break;
        if(rt->rt_flags & RTF_GATEWAY)
            continue;
    }
    if(src_addr != NULL)
        *src_addr = rt->rt_dev->pa_addr;
    if(daddr == rt->rt_dev->pa_addr)
        if((rt = rt_loopback) == NULL)
            goto no_route;
    rt->rt_use++;
    return rt;
no_route:
    return NULL;
}

struct rtable* ip_rt_local(unsigned long daddr, struct options *opt, unsigned long *src_addr)
{
    struct rtable *rt;
    for(rt = rt_base; rt!= NULL; rt = rt->rt_next)
    {
        if(rt->rt_flags & RTF_GATEWAY)
            continue;
        if(!((daddr & rt->rt_mask) ^ rt->rt_dst))
            break;
    }

    if(src_addr != NULL)
        *src_addr = rt->rt_dev->pa_addr;
    if(daddr == rt->rt_dev->pa_addr)
        if((rt = rt_loopback) == NULL)
            goto no_route;
    rt->rt_use++;
    return rt;
no_route:
    return NULL;
}


int ip_rt_ioctl(unsigned int cmd, void *arg)
{
    switch(cmd)
    {
        case  	SIOCADDRT:
        case 	SIOCDELRT:
            return -EINVAL;	
    }
    return -EINVAL;
}
