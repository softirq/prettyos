#include "type.h"
#include "const.h"
#include "traps.h"
//#include "string.h"
#include "sched.h"
#include "tty.h"
#include "console.h"
#include "global.h"
//#include "kernel.h"
#include "lib.h"
#include "mm.h"
#include "linux/timer.h"
#include "errno.h"
#include "stddef.h"
#include "asm-i386/byteorder.h"
#include "printf.h"

#include "linux/netdevice.h"
#include "linux/if_ether.h"
#include "linux/if_arp.h"
#include "linux/tcp.h"

#ifndef _NOTIFIER_H
#define _NOTIFIER_H
#include "linux/notifier.h"
#endif

#include "linux/if.h"
#include "linux/sockios.h"
#include "sock.h"

int arp_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt);

struct arp_table 
{
    struct arp_table *next;
    unsigned long last_used;
    unsigned int flags;
    unsigned long ip;
    unsigned long mask;  //netmask
    unsigned char ha[MAX_ADDR_LEN];
    unsigned char hlen;
    unsigned short htype;
    struct net_device *dev;
    struct timer_list 	timer;
    unsigned int retries;
    struct sk_buff_head 	skh;
};

#define  	DEF_ARP_NETMASK 	(~0)
#define 	ARP_TIMEOUT 		(600*HZ)

#define 	ARP_RES_TIME 		(250 * (HZ /10))
#define 	ARP_MAX_TRIES 		3
#define 	ARP_TABLE_SIZE 		16

#define 	FULL_ARP_TABLE_SIZE 	(ARP_TABLE_SIZE+1)


#define  	HASH(addr) 			(addr & (ARP_TABLE_SIZE - 1))

struct arp_table *arp_tables[FULL_ARP_TABLE_SIZE] =
{
    NULL,
};

static struct packet_type arp_packet_type = 
{
    0,
    NULL,
    arp_rcv,
    NULL,
    NULL,
};

#define 	ARP_CHECK_INTERVA 	(60*HZ)
static void arp_check_expire(unsigned long );

static struct timer_list arp_timer =
{
    NULL,
    NULL,
    ARP_CHECK_INTERVA,
    0L,
    &arp_check_expire
};

static void arp_check_expire(unsigned long dummy)
{
    int i;
    unsigned long now = jiffies;

    struct arp_table *entry;
    struct arp_table **pentry;
    for(i = 0;i < FULL_ARP_TABLE_SIZE; i++)
    {
        pentry = &arp_tables[i];
        while((entry = *pentry) != NULL)
        {
            if((now - entry->last_used) > ARP_TIMEOUT)
            {
                *pentry = entry->next;
                del_timer(&entry->timer);
            }

            else
                pentry = &entry->next;
        }

    }
    del_timer(&arp_timer);
    arp_timer.expires = ARP_CHECK_INTERVA;
    add_timer(&arp_timer);
}

static void arp_release_entry(struct arp_table *entry)
{
    struct sk_buff *skb;
    while((skb = skb_dequeue(&entry->skh)) != NULL)
    {
        dev_kfree_skb(skb,FREE_WRITE);

    }

    del_timer(&entry->timer);

    //	kfree(entry,sizeof(struct arp_table));
    return;
}

/*
 * purge a net_device from ARP queue
 * 
 * */

int arp_device_event(unsigned long event, void *ptr)
{
    struct net_device *dev = (struct net_device *)ptr;
    int i;

    if(event != NETDEV_DOWN)
        return NOTIFY_DONE;

    for(i = 0;i < FULL_ARP_TABLE_SIZE;i++)
    {
        struct arp_table *entry;
        struct arp_table **pentry = &arp_tables[i];
        while((entry = *pentry) != NULL)
        {
            if(entry->dev == dev)
            {
                *pentry = entry->next;
                del_timer(&entry->timer);
                //							kfree_s(entry,sizeof(struct arp_table));
            }
            else
                pentry =&entry->next;
        }
    }
    return NOTIFY_DONE;
}

/*
 * create and send an arp packet. if dest_hw == NULL,create a broadcast message.
 */
void arp_send(int type,int ptype,unsigned long dest_ip,struct net_device *dev,unsigned long src_ip,unsigned char *dest_hw,unsigned char *src_hw)
{
    struct sk_buff *skb;
    struct arphdr *arph;
    unsigned char *arp_ptr;

    if(dev->flags & IFF_NOARP)
        return;
    skb = alloc_skb((sizeof(struct arphdr) + 2 *(dev->addr_len + IP_ALEN) + dev->hard_header_len),GFP_ATOMIC);
    if(skb == NULL)
    {
        printk("ARP:no memory to send an arp packet\n");
        return;
    }

    skb->len = sizeof(struct arphdr) + dev->hard_header_len + 2 * (dev->addr_len + IP_ALEN);
    skb->arp = 1;
    skb->dev = dev;
    skb->free = 1;

    //build eth header
    dev->hard_header(skb->data,dev,ptype,dest_hw?dest_hw:dev->broadcast,src_hw?src_hw:NULL,skb->len,skb);

    //build arp header
    arph = (struct arphdr*)(skb->data + dev->hard_header_len);
    arph->ar_hwt = htons(dev->type);
    arph->ar_prot = htons(ETH_P_IP);
    arph->ar_hlen = dev->addr_len;
    arph->ar_plen = IP_ALEN;
    arph->ar_op = htons(type);

    //build arp data

    arp_ptr = (unsigned char *)(arph + 1);
    memncpy(arp_ptr,src_hw,dev->addr_len);
    arp_ptr += dev->addr_len;
    memncpy(arp_ptr,(char *)&src_ip,IP_ALEN);
    arp_ptr += IP_ALEN;


    if(dest_hw != NULL)
        memncpy(arp_ptr,dest_hw,dev->addr_len);
    else
        memset(arp_ptr,0,dev->addr_len);
    arp_ptr += dev->addr_len;
    memncpy(arp_ptr,(char *)&dest_ip,IP_ALEN);

    //		dev_queue_ximit(skb,dev,0);

    return;
}

/*
 * resend a request or give it up and free the entry
 */
static void arp_expire_request(unsigned long arg)
{
    struct arp_table *entry = (struct arp_table *)arg;
    struct arp_table **pentry;
    unsigned long hash;

    /*
     * it's a valid entry
     */
    if(entry->flags & ATF_COM)
    {
        return;
    }

    if(--entry->retries > 0)
    {
        unsigned long ip = entry->ip;
        struct net_device *dev = entry->dev;

        del_timer(&entry->timer);
        entry->timer.expires = ARP_RES_TIME;
        add_timer(&entry->timer);
        arp_send(ARPOP_REQUEST,ETH_P_ARP,ip,dev,dev->pa_addr,NULL,dev->dev_addr);
        return;
    }

    hash = HASH(entry->ip);
    /*   arp proxy
     *
     if(entry->flags & ATF_PUBL)
     pentry = &ary->tables[PROXY_HASH];
     else
     */
    pentry = &arp_tables[hash];
    while(*pentry != NULL)
    {
        if(*pentry == entry)
        {
            *pentry = entry->next;
            del_timer(&entry->timer);
            arp_release_entry(entry);
            return;
        }
        pentry = &(*pentry)->next;
    }
    return;
}

static void arp_send_q(struct arp_table *entry, unsigned char *hw_dest)
{
    struct sk_buff *skb;
    //	unsigned long flags;

    if(!(entry->flags & ATF_COM))
    {
        printk("arp_send_q : incomplete entry for %s\n",entry->ip);
        return;
    }

    while((skb = skb_dequeue(&entry->skh)) != NULL)
    {
        if(skb->dev->rebuild_header(skb->data,skb->dev,skb->raddr,skb) == 0)
        {
            skb->arp = 1;
            if(skb->sk == NULL)
                dev_queue_xmit(skb,skb->dev,0); //send a packet
            else
                dev_queue_xmit(skb,skb->dev,skb->sk->priority);
        }
        else
        {
            printk("arp_send_q : active entry %s\n",entry->ip);
            printk("arp_send_q : failed to find %s\n",skb->raddr);
        }


    }

}

void arp_destory(unsigned long ip_addr,int force)
{
    struct arp_table **pentry;
    struct arp_table *entry;
    unsigned long hash = HASH(ip_addr);
    pentry =  &(arp_tables[hash]);

    while((entry = *pentry) != NULL)
    {
        if(entry->ip == ip_addr)
        {
            if((entry->flags&ATF_PERM) && !force)
                return;

            *pentry = entry->next;
            del_timer(&entry->timer);
            arp_release_entry(entry);
        }
    }
    return;
}

/*
 * receive an arp request by the net_device net_device layer
 * */


int arp_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt)
{
    unsigned long hash;
    struct arphdr *arp = (struct arphdr *)skb->h.raw;
    unsigned char *arp_ptr = (unsigned char *)(arp + 1);
    struct arp_table *entry;
    //		struct arp_table **pentry;

    int addr_hint,hlen,htype;
    long sip,dip; 		//source ip  dest ip
    unsigned char *sha,*dha;


    /*
     *check arp header
     */ 

    if(arp->ar_hlen != dev->addr_len || 
            arp->ar_hwt != htons(dev->type) ||
            dev->flags & IFF_NOARP ||
            arp->ar_plen != IP_ALEN)
    {
        return 0;
    }

    /*
     * check for  protocol
     */
    switch(dev->type)
    {
        case ARPHRD_AX25:
            break;
        case ARPHRD_ETHER:
            if(arp->ar_prot != htons(ETH_P_IP))
            {
                //		kfree_skb(skb,FREE_READ);
                return 0;
            }
            break;
        default:
            printk("ARP : dev->type mangled!\n");
            //	kfree_skb(skb,FREE_READ);
            return 0;

    }

    hlen = dev->addr_len;
    htype = dev->type;

    sha = arp_ptr;
    *(sha + hlen) = '\0';
    arp_ptr += hlen;
    memncpy(&sip,(char *)arp_ptr,IP_ALEN);
    arp_ptr += IP_ALEN;
    dha = arp_ptr;
    *(dha + hlen) = '\0';
    memncpy(&dip,(char *)arp_ptr,4);

    //bad reqeust from 127.0.0.1
    if(dip == INADDR_LOOPBACK)	
    {
        //				kfree_skb(skb,FREE_READ);
        return 0;
    }
    /*
     * arp request packet
     */

    addr_hint = ip_chk_addr(dip);
    if(arp->ar_op == htons(ARPOP_REPLY))
    {
        if(addr_hint != IS_MYADDR)
            //kfree_skb(skb,FREE_READ);
            return 0;
    }
    /*
     * arp request packet
     */
    else
    {
        if(dip != dev->pa_addr)
        {
            //		kfree_skb(skb,FREE_READ);
            return 0;
        }
        else
            /* replay an arp request*/
            arp_send(ARPOP_REPLY,ETH_P_ARP,sip,dev,dip,sha,dev->dev_addr);
    }

    hash = HASH(sip);

    for(entry = arp_tables[hash];entry;entry=entry->next)
        if(entry->ip == sip && entry->htype == htype)
            break;
    if(entry)
    {
        memncpy(entry->ha,sha,hlen);
        entry->hlen = hlen;
        entry->last_used = jiffies;
        if((entry->flags & ATF_COM))
        {
            del_timer(&entry->timer);
            entry->flags |= ATF_COM;
            arp_send_q(entry,sha);
        }
    }
    else
    {
        //			entry = (struct arp_table *)kmalloc(sizeof(struct arp_table),GFP_ATOMIC);
        if(entry == NULL)
        {
            printk("ARP : no memory for new entry\n");
            //		kfree_skb(skb,FREE_READ);
            return 0;
        }
        entry->mask = DEF_ARP_NETMASK;
        entry->ip = sip;
        entry->hlen = hlen;
        entry->htype = htype;
        entry->flags = ATF_COM;
        init_timer(&entry->timer);
        memncpy(entry->ha,sha,hlen);
        entry->last_used = jiffies;
        skb_queue_head_init(entry->skh);
        entry->next = arp_tables[hash];
        arp_tables[hash] = entry;
    }

    //		kfree_skb(skb,FREE_READ);
    return 0;
}

static struct arp_table * arp_lookup(unsigned long paddr)
{
    struct arp_table *entry;
    unsigned long hash = HASH(paddr);

    for(entry = arp_tables[hash];entry != NULL;entry++)
    {
        if(entry->ip == paddr)
        {
            break;
        }
    }
    return entry;
}

int arp_find(unsigned char *haddr,unsigned long paddr,struct net_device *dev,unsigned saddr,struct sk_buff *skb)
{
    struct arp_table *entry;
    unsigned long hash;
    unsigned long daddr;
    switch(ip_chk_addr(paddr))
    {
        case IS_MYADDR:
            printk("ARP : arp called form own IP address]n");
            memncpy(haddr,dev->dev_addr,dev->addr_len);
            skb->arp = 1;
            return 0;

        case IS_MULTICAST:
            if(dev->type == ARPHRD_ETHER)
            {
                haddr[0] = 0x01;
                haddr[1] = 0x00;
                haddr[2] = 0x5e;
                daddr = ntohl(paddr);
                haddr[5] = daddr & 0xff;
                daddr = daddr >> 8;
                haddr[4] = daddr & 0xff;
                daddr = daddr >> 8;
                haddr[3] = daddr & 0x7f;
                return 0;
            }
        case IS_BROADCAST:
            memncpy(haddr,dev->broadcast,dev->addr_len);
            skb->arp = 1;
            return 0;

    }

    hash = HASH(paddr);
    entry = arp_lookup(paddr);
    if(entry != NULL)
    {
        if(!(entry->flags & ATF_COM))
        {
            if(skb != NULL)
            {
                skb_queue_tail(&entry->skh,skb);
            }
            return 0;
        }
        entry->last_used = jiffies;
        memncpy(haddr,entry->ha,dev->addr_len);
        if(skb)
            skb->arp = 1;
        return 0;
    }

    //			entry = (struct arp_table *)kmalloc(sizeof(struct arp_table),GFP_ATOMIC);
    if(entry != NULL)
    {


        entry->mask = DEF_ARP_NETMASK;
        entry->ip = paddr;
        entry->hlen = dev->addr_len;
        entry->htype = dev->type;
        entry->flags = 0;

        memset(entry->ha,0,dev->addr_len);
        entry->dev = dev;
        init_timer(&entry->timer);
        entry->timer.function = arp_expire_request;
        entry->timer.data = (unsigned long )entry;
        entry->timer.expires = ARP_RES_TIME;
        entry->next = arp_tables[hash];
        arp_tables[hash]->next = entry;
        add_timer(&entry->timer);
        entry->retries = ARP_MAX_TRIES;
        skb_queue_head_init(&entry->skh);
        if(skb != NULL)
        {
            skb_queue_tail(&entry->skh,skb);

        }
    }
    else
    {
        //	if(skb != NULL;&& skb->free)	
        //					kfree_skb(skb,FREE_WRITE);

    }
    arp_send(ARPOP_REQUEST,ETH_P_ARP,paddr,dev,saddr,NULL,dev->dev_addr);

    return 0;
}

/*
 * set an arp entry
 */
static int arp_req_set(struct arpreq *req)
{
    struct arpreq arq;
    struct arp_table *entry;
    struct sockaddr_in *sin;
    int htype,hlen;
    unsigned long ip;
    unsigned long hash;
    //		struct rtable *rt;

    memncpy(&arq,req,sizeof(arq));
    switch(arq.arp_pa.sa_family)
    {
        case ARPHRD_ETHER:
            htype = ARPHRD_ETHER;
            hlen = ETH_ALEN;
            break;
        default:
            return -EPFNOSUPPORT;
    }
    sin = (struct sockaddr_in*)&arq.arp_pa;
    ip = sin->sin_addr.s_addr;
    if(ip == 0)
    {
        printk("ARP : request protocol address is  0.0.0.0\n");
        return -EINVAL;
    }

    /*
       rt = ip_rt_route(ip,NULL,NULL);
       if(rt == NULL)
       return -ENETUNREACH;
       */

    entry = arp_lookup(ip);
    hash = HASH(ip);

    if(entry == NULL)
    {
        //				entry = (struct arp_table *)kmalloc(sizeof(struct arp_table),GFP_ATOMIC);
        if(entry == NULL)
        {
            return -ENOMEM;
        }

        entry->ip = ip;
        entry->hlen = hlen;
        entry->htype = htype;
        init_timer(&entry->timer);
        entry->next = arp_tables[hash];
        arp_tables[hash] = entry;
        skb_queue_head_init(&entry->skh);
    }

    memncpy(&entry->ha,&arq.arp_ha.sa_data,hlen);
    entry->last_used = jiffies;
    entry->flags = arq.arp_flags | ATF_COM;
    entry->mask = DEF_ARP_NETMASK;
    //		entry->dev = rt->rt_dev;
    return 0;
}

static int arp_req_get(struct arpreq *req)
{
    struct arpreq r;
    struct arp_table *entry;
    struct sockaddr_in *sin;

    memncpy(&r,req,sizeof(r));

    if(r.arp_pa.sa_family != AF_INET)
        return -EPFNOSUPPORT;

    sin = (struct sockaddr_in *)&r.arp_pa;
    entry = arp_lookup(sin->sin_addr.s_addr);

    if(entry == NULL)
    {
        return -ENXIO;
    }

    memncpy(r.arp_ha.sa_data,&entry->ha,entry->hlen);
    r.arp_ha.sa_family = entry->htype,
        r.arp_flags = entry->flags;
    memncpy(req,&r,sizeof(r));
    return 0;
}


int arp_ioctl(unsigned int cmd, void *arg)
{
    struct arpreq 	r;
    struct sockaddr_in *sin;
    //		int err;

    switch(cmd)
    {  //delete arp entry
        case SIOCDARP:
            //			if(!suser())
            //					return -EPERM;
            memncpy(&r,arg,sizeof(r));
            if(r.arp_pa.sa_family != AF_INET)
                return -EPFNOSUPPORT;
            sin = (struct sockaddr_in*)&r.arp_pa;
            arp_destory(sin->sin_addr.s_addr,1);
            return 0;
        case SIOCGARP:
            return arp_req_get((struct arpreq*)arg);
        case SIOCSARP:
            return arp_req_set((struct arpreq*)arg);
        default:
            return -EINVAL;

    }
    return 0;
}


static struct notifier_block arp_dev_notifier =
{
    arp_device_event,
    NULL,
    0
};

void arp_init()
{
    arp_packet_type.type = htons(ETH_P_ARP);
    dev_add_pack(&arp_packet_type);
    add_timer(&arp_timer);
    register_netdevice_notifier(&arp_dev_notifier);
}
