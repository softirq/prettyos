#include "type.h"
#include "stddef.h"
#include "errno.h"
#include "string.h"
#include "asm-i386/byteorder.h"

#include "asm-i386/system.h"
#include "asm-i386/param.h"

#include "linux/notifier.h"
#include "linux/netdevice.h"
#include "linux/if_ether.h"
#include "linux/if.h"
#include "linux/timer.h"
#include "linux/tcp.h"

#include "ip.h"
#include "sock.h"
#include "route.h"
#include "printf.h"

#define 	LOOPBACK(x) 	(((x) & htonl(0xff000000)) == htonl(0x7f000000))

int ip_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt);
void ip_queue_xmit(struct sock *sk, struct net_device *dev, struct sk_buff *skb, int free);

static int ip_rt_event(unsigned long event, void *val)
{
    if(event == NETDEV_DOWN)
    {
        ip_rt_flush(val);
        return NOTIFY_DONE;
    }
    return 0;
}

static struct packet_type ip_packet_type = 
{
    0,
    NULL,
    ip_rcv,
    NULL,
    NULL
};

struct notifier_block ip_rt_notifier =
{
    ip_rt_event,
    NULL,
    0
};


int ip_ioctl(struct sock *sk, int cmd, unsigned long arg)
{
    switch(cmd)
    {
        default:
            return -EINVAL;
    }
}

#if 0
static int build_options(struct iphdr *iph, struct options *opt)
{
    unsigned char *ptr;
    ptr = (unsigned char *)(iph + 1);
    *ptr = 0;
    return 4;
}
#endif


/*
 * build ether header
 */
static int ip_build_eth_header(struct sk_buff *skb, unsigned long daddr, int len, struct net_device *dev, unsigned long saddr)
{
    int mac = 0;
    skb->dev = dev;
    skb->arp = 1;

    if(dev->hard_header)
    {
        mac = dev->hard_header(skb->data, dev, ETH_P_IP, NULL, NULL, len, skb);
        if(mac < 0)
        {
            mac = ~mac;
            skb->arp = 0;
            skb->raddr = daddr;
        }
    }
    return mac;

}

/*
 * build ether and ip header
 */
int ip_build_header (struct sk_buff *skb, unsigned long saddr, unsigned long daddr, struct net_device **dev, int type, struct options *opt, int len, int tos, int ttl)
{
    struct rtable 	*rt;
    struct options 	optval;
    unsigned char 	*buff;
    int 	tmp;
    unsigned long 	raddr;
    unsigned long 	src;
    struct iphdr *iph;

    buff = skb->data;

    if(*dev == NULL)
    {
        if(skb->localroute)
            rt = ip_rt_local(daddr, &optval, &src);
        else
            rt = ip_rt_route(daddr, &optval, &src);
        if(rt == NULL)
            return -ENETUNREACH;

        *dev = rt->rt_dev;

        if(LOOPBACK(saddr) && !LOOPBACK(daddr))
            saddr = src;
        raddr = rt->rt_gateway;
        opt = &optval;
    }
    else
    {
        if(skb->localroute)
            rt = ip_rt_local(daddr, &optval, &src);
        else
            rt = ip_rt_route(daddr, &optval, &src);
        if((LOOPBACK(saddr)) && !LOOPBACK(daddr))
            saddr = src;

        raddr = (rt == NULL)?0:rt->rt_gateway;
    }

    if(saddr == 0)
        saddr = src;
    if(raddr == 0)
        raddr = daddr;
    tmp = ip_build_eth_header(skb, raddr, len, *dev, saddr);
    buff += tmp;
    len -= tmp;
    skb->dev = *dev;
    skb->saddr = saddr;

    if(skb->sk)
        skb->sk->saddr = saddr;

    if(type == IPPROTO_RAW)
        return tmp;

    iph = (struct iphdr*)buff;
    iph->version = 4;
    iph->tot_len  = tos;
    iph->flags_off = 0;
    iph->ttl = ttl;
    iph->daddr = daddr;
    iph->saddr = saddr;
    iph->protocol = type;
    iph->ihl = 5;
    skb->ip_hdr = iph;

    return (20 + tmp);
}

static int do_options(struct iphdr *iph, struct options *opt)
{
    return 0;
}

unsigned short ip_compute_chksum(unsigned char *buff, int len)
{
    return 0;
}

void ip_send_checksum(struct iphdr *iph)
{
    iph->chksum = 0;
    return;
}

static struct ipfq *ipfrag_queue = NULL;

static struct ipfrag* ip_frag_create(int offset, int end, struct sk_buff *skb, unsigned char *ptr)
{
    struct ipfrag *fp;
    //		fp = (struct ipfrag*)kmalloc(sizeof(struct ipfrag*),GFP_ATOMIC);
    if(fp == NULL)
    {
        printk("IP : frag_create : no enough memory\n");
        return NULL;
    }
    memset((char *)fp,0,sizeof(struct ipfrag));
    fp->offset = offset;
    fp->end = end;
    fp->len = end - offset;
    fp->skb = skb;
    fp->ptr = ptr;
    return fp;
    return NULL;
}

/*
 * find the queue for the ip datagram
 */
static struct ipfq *ip_frq_find (struct iphdr *iph)
{
    struct ipfq  	*fq;

    cli();

    for(fq = ipfrag_queue; fq != NULL; fq = fq->next)
    {
        if(iph->id == fq->iph->id && iph->saddr == fq->iph->saddr && iph->daddr == fq->iph->daddr && \
                iph->protocol == fq->iph->protocol)
        {
            del_timer(&fq->timer);
            sti();
            return fq;
        }
    }

    return NULL;
}

static void ip_frq_free(struct ipfq *fq)
{
    struct ipfrag 	*ifg;
    struct ipfrag 	*ifg_next;
    del_timer(&fq->timer);
    cli();
    if(fq->prev == NULL)
    {
        ipfrag_queue = fq->next;
        if(ipfrag_queue != NULL)
            ipfrag_queue->prev = NULL;
    }
    else
    {
        fq->prev->next = fq->next;
        if(fq->next != 	NULL)
            fq->next->prev = fq->prev;
    }
    ifg = fq->fragments;
    while(ifg != NULL)
    {
        ifg_next = ifg_next;
        //				kfree_skb(ifg->skb,FR_READ);
        //				kfree_s(ifg,sizeof(struct ipfrag));
        ifg = ifg_next;
    }
    //		kfree_s(fg->mac, fg->maclen);
    //		kfree_s(fg->iph, fg->ihlen + 8);
    //		kfree_s(fg,sizeof(struct ifq));

    sti();
    return;
}

static void ip_frq_expire(unsigned long arg)
{
    struct ipfq *ifq;
    ifq = (struct ipfq*)arg;
    if(ifq->fragments != NULL)
    {
        //				icmp_send(ifq->fragments->skb, ICMP_TIME_EXCEEDED, ICMP_EXC_FRAGTIME, 0,ifq->dev);
    }
    ip_frq_free(ifq);
}

static struct ipfq* ip_frq_create(struct sk_buff *skb, struct iphdr *iph, struct net_device *dev)
{
    struct ipfq *ifq;
    int maclen;
    int ihlen;

    //		ifq = (struct ipfq)kmalloc(sizeof(struct ipfq),GFP_ATOMIC);
    if(ifq == NULL)
    {
        printk("ifq create no memory alloc\n");
        return NULL;
    }
    memset((char *)ifq, 0, sizeof(struct ipfq));

    maclen = ((unsigned long)iph - (unsigned long)skb->data);
    //		ifq->mac = (unsigned char*)kmalloc(sizeof(maclen));
    if(ifq->mac == NULL)
    {
        printk("ifq create no memory alloc\n");
        //				kfree_s(ifq, sizeof(struct ipfq));
        return NULL;
    }

    ihlen = (iph->ihl * sizeof(unsigned long));
    //		ifq->iph = (struct iphdr*)kmalloc(ihlen);
    if(ifq->iph == NULL)
    {
        printk("ifq create no memory alloc\n");
        //				kfree_s(ifq->mac,maclen);
        //				kfree_s(ifq, sizeof(struct ipfq));
        return NULL;
    }

    memncpy((char *)ifq->mac, (char *)skb->data,maclen);
    memncpy((char *)ifq->iph, (char *)iph, ihlen + 8);
    ifq->len = 0;
    ifq->ihlen = ihlen;
    ifq->maclen = maclen;
    ifq->fragments = NULL;
    ifq->dev = dev;

    ifq->timer.expires = IP_FRAG_TIME;
    ifq->timer.data = (unsigned long)ifq;
    ifq->timer.function = ip_frq_expire;
    add_timer(&ifq->timer);

    ifq->prev = NULL;
    cli();

    ifq->next = ipfrag_queue;
    if(ifq->next != NULL)
        ifq->next->prev = ifq;
    ipfrag_queue = ifq;
    sti();
    return ifq;
}

/*
 * see if all fragments is complete
 * return 1 -- complete ; return 0 -- miss
 */

static int ip_frq_done(struct ipfq *ifq)
{
    struct ipfrag 	*frag;
    int offset;
    if(ifq->len == 0)
        return 0;
    frag = ifq->fragments;
    offset = 0;

    while(frag != NULL)
    {
        if(frag->offset > offset)
            return 0;
        offset = frag->end;
        frag = frag->next;
    }
    return 1;
}

/*
 * build a new ip datagram from all its fragments
 */
static struct sk_buff *ip_frq_glue(struct ipfq *ifq)
{
    struct sk_buff 	*skb;
    struct iphdr 	*iph;
    struct ipfrag 	*ifg;
    unsigned char *ptr;

    int len, count;

    len = ifq->maclen + ifq->ihlen + ifq->len;

    //		skb = (struct sk_buff*)kmalloc(len);
    if(skb == NULL)
    {
        printk("IP : ip_frag_glue : no memory for glue\n ");
        ip_frq_free(ifq);
        return NULL;
    }

    skb->len = (len - ifq->maclen);
    skb->h.raw = skb->data;
    skb->free = 1;

    ptr = (unsigned char *)skb->h.raw;
    memncpy(ptr, (unsigned char*)ifq->mac, ifq->maclen);
    ptr += ifq->maclen;
    memncpy(ptr, (unsigned char*)ifq->iph, ifq->ihlen);
    ptr += ifq->ihlen;
    skb->h.raw += ifq->maclen;

    count = 0;
    ifg = ifq->fragments;
    while(ifg != NULL)
    {
        if((count + ifg->len) >  skb->len)
        {
            printk("invalid fragment list:fragments over size. \n");
            ip_frq_free(ifq);
            //						kfree_skb(skb,FREE_WRITE);
            return NULL;					
        }

        memncpy((ptr + ifg->offset),ifg->ptr, ifg->len);
        count += ifg->len;
        ifg = ifg->next;
    }
    ip_frq_free(ifq);
    iph = skb->h.iph;
    iph->flags_off = 0;
    iph->tot_len = htons(iph->ihl * sizeof(unsigned long) + count);
    skb->ip_hdr = iph;

    return NULL;
}

/*
 * process an incoming ip datagarm fragment
 */
static struct sk_buff* ip_defrag(struct iphdr *iph, struct sk_buff *skb, struct net_device *dev)
{
    struct ipfrag 	*prev, *next;
    struct ipfrag 	*tfp;
    struct ipfq 	*ifq;

    //	struct sk_buff  *skbuff;

    int flags, offset;
    int i, ihl,end;
    unsigned char *ptr;

    ifq = ip_frq_find(iph);

    offset = ntohs(iph->flags_off);
    flags = offset & IP_FLAGS;
    offset = offset & IP_OFFSET;

    if(((flags & IP_MF) == 0) && offset == 0)
    {
        if(ifq == NULL)
            ip_frq_free(ifq);
        return skb;
    }
    offset <<= 3;

    if(ifq != NULL)
    {
        del_timer(&ifq->timer);
        ifq->timer.expires = IP_FRAG_TIME;
        ifq->timer.data = (unsigned long)ifq;
        ifq->timer.function = ip_frq_expire;
        add_timer(&ifq->timer);
    }
    else
    {
        if((ifq = ip_frq_create(skb, iph, dev)) == NULL)
        {
            skb->sk = NULL;
            //				kfree_skb(skb, FREE_READ);
            return NULL;
        }
    }

    ihl = (iph->ihl * sizeof(unsigned long));
    end = offset + ntohs(iph->tot_len) - ihl;
    ptr = skb->data + dev->hard_header_len + ihl;
    if((flags & IP_MF) == 0)
        ifq->len = end;
    prev = NULL;
    for(next = ifq->fragments; next != NULL; next = next->next)
    {
        if(next->offset > offset)
            break;
        prev = next;
    }

    if(prev != NULL && offset < prev->end)
    {
        i = prev->end - offset;
        offset += i;
        ptr += i;
    }

    for(; next != NULL; next = tfp)
    {
        tfp = next->next;
        if(next->offset >= end)
            break;
        i = end - next->offset;
        next->len -= i;
        next->offset += i;
        next->ptr += i;
        if(next-> len <= 0)
        {
            if(next->prev != NULL)
                next->prev->next = next->next;
            else
                ifq->fragments = next->next;
        }
        if(tfp != NULL)
            tfp->prev = next->prev;
        //	kfree_skb(next->skb, FREE_READ);
        //	kfree_s(next, sizeof(struct ipfrag));
    }

    tfp = NULL;
    tfp = ip_frag_create(offset, end, skb, ptr);
    if(!tfp)
    {
        skb->sk = NULL;
        //				kfree_skb(skb, FREE_READ);
        return NULL;
    }

    tfp->prev = prev;
    tfp->next = next;
    if(prev != NULL)
        prev->next = tfp;
    else
        ifq->fragments = tfp;

    if(next != NULL)
        next->prev = tfp;


    if(ip_frq_done(ifq))
    {
        return ip_frq_glue(ifq);
    }
    return NULL;
}

/*
 * This IP datagram is too large to be sent in one piece,break it up into smaller pieces
 */
void ip_fragment(struct sock *sk, struct sk_buff *skb, struct net_device *dev, int is_flag)
{
    struct iphdr *iph;
    unsigned char *ptr;
    unsigned char *raw;
    int left, mtu, hlen, len;
    int offset;
    unsigned long flags;

    struct sk_buff *skb2;

    raw = skb->data;
    iph = (struct iphdr*)(raw + dev->hard_header_len);
    skb->ip_hdr = iph;
    hlen = (iph->ihl * sizeof(unsigned long));
    left = ntohs(iph->tot_len) - hlen;
    hlen += dev->hard_header_len;
    mtu = (dev->mtu - hlen);
    ptr = raw + hlen;

    if(ntohs(iph->flags_off) & IP_DF)
    {
        //			icmp_send(skb,ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED,dev->mtu,dev);
        return;	
    }
    if(mtu < 8)
    {
        //			icmp_send(skb,ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED,dev->mtu,dev);
        return;	
    }

    if(is_flag & 3)
        offset = (ntohs(iph->flags_off) & IP_OFFSET) << 3;
    else
        offset = 0;
    while(left > 0)
    {
        len = left;
        if(len > mtu)
        {
            len = mtu;
        }
        // not the last piece
        if(len < left)
        {
            len = len/8;
            len *= 8;
        }
        /*			
                    if((skb2 = alloc_skb(len + hlen, GFP_ATOMIC)) == NULL)
                    {
                    printk("IP fragment : no  memory for new fragment\n");
                    return;
                    }
                    */
        skb2->arp = skb->arp;
        if(skb->free == 0)
            printk("IP fragment : free = 1\n");
        skb2->free = 1;
        skb->len = len + hlen;
        skb->h.raw = (char *)skb2->data;

        save_flags(flags);
        if(sk)
        {
            cli();
            skb2->sk = sk;
        }
        restore_flags(flags);

        //copy the packet header 
        memncpy(skb2->h.raw, raw, hlen);
        // coppy the ip datagram
        memncpy(skb2->h.raw + hlen, ptr, len);

        left -= len;

        iph = (struct iphdr *)(skb2->h.raw);
        iph->flags_off = htons(offset >> 3);
        if(left > 0 || (is_flag & 1))
            iph->flags_off |= htons(IP_MF);

        skb2->h.raw += dev->hard_header_len;
        ptr += len;
        offset += len;

        ip_queue_xmit(sk, dev, skb2, 2);
    }
}

static void ip_forward(struct sk_buff *skb, struct net_device *dev, int is_flag)
{
    struct net_device 	*dev2;
    struct sk_buff 	*skb2;
    struct iphdr 	*iph;

    struct rtable  	*rt;
    unsigned char 	*ptr;
    unsigned long 	raddr;

    iph = skb->h.iph;
    iph->ttl--;
    if(iph->ttl <= 0)
    {
        //				icmp_send(skb, ICMP_TIME_EXCEEDED, ICMP_EXC_TTL, 0, dev);
        return;
    }
    ip_send_checksum(iph);
    rt = ip_rt_route(iph->daddr, NULL, NULL);
    if(rt == NULL)
    {
        //icmp_send(skb, ICMP_DEST_UNREACH, ICMP_NET_UNREACH, 0, dev);
        return;
    }

    raddr = rt->rt_gateway;

    if(raddr != 0)
    {
        rt = ip_rt_route(raddr, NULL, NULL);
        if(rt == NULL)
        {
            //icmp_send(skb, ICMP_DEST_UNREACH, ICMP_NET_UNREACH, 0, dev);
            return;
        }
        if(rt->rt_gateway != 0)
            raddr = rt->rt_gateway;
    }
    else
        raddr = iph->daddr;

    dev2 = rt->rt_dev;

    if(dev == dev2 && (iph->saddr & dev->pa_mask) == (iph->daddr & dev->pa_mask))
    {
        //	icmp_send(skb, ICMP_REDIRECT, ICMP_REDIR_HOST, 0, dev);
    }

    if(dev2->flags * IFF_UP)
    {
        //			skb2 = alloc_skb(dev2->hard_header_len + skb->len, GFP_ATOMIC);
        if(skb2 == NULL)
        {
            printk(" no memory available for ip forward\n");
            return;
        }

        ptr = skb2->data;
        skb2->free = 1;
        skb2->len = skb->len + dev2->hard_header_len;
        skb2->h.raw = ptr;

        memncpy(ptr + dev2->hard_header_len, skb->h.raw, skb->len);
        ip_build_eth_header(skb2, raddr, skb->len, dev2 ,dev2->pa_addr);

        if(skb2->len > dev2->mtu + dev2->hard_header_len)
        {
            ip_fragment(NULL, skb2, dev2, is_flag);
            //		kfree_skb(skb2, FREE_WRITE);
        }
        else
        {
            if(iph->tos & IPTOS_LOWDELAY)
                dev_queue_xmit(skb2, dev2, SOPRI_INTERACTIVE);
            else if(iph->tos & IPTOS_THROUGHPUT)
                dev_queue_xmit(skb2, dev2, SOPRI_BACKGROUND);
            else
                dev_queue_xmit(skb2, dev2, SOPRI_NORMAL);
        }
    }
    return;
}

int ip_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pack_t)
{
    struct options 	opt;
    struct iphdr *iph = skb->h.iph;
    unsigned char hash;
    unsigned char opts_p;
    struct inet_protocol *ipprot;
    int is_flag = 0;

    int brd = IS_MYADDR;

    skb->ip_hdr = iph;

    if(skb->len < sizeof(struct iphdr) || iph->ihl < 5 || iph->version != 4 || ip_fast_cksum((unsigned char *)iph,iph->ihl) != 0)
    {
        //				kfree_skb(skb,FREE_WRITE);
        return 0;
    }

    skb->len = ntohs(iph->tot_len);
    if(iph->ihl != 5)
    {
        memset((char *)&opt, 0 ,sizeof(struct options));
        if(do_options(iph, &opt) != 0)
            return 0;
        opts_p = 1;
    }

    if(iph->flags_off)
    {
        if(iph->flags_off & IP_MF)
            is_flag |= 1;
        if(iph->flags_off & IP_OFFSET)
            is_flag |= 2;
    }

    if(iph->daddr != skb->dev->pa_addr && (brd = ip_chk_addr(iph->daddr)) == 0)
    {
        if(skb->pkt_type != PACKET_HOST || brd == IS_BROADCAST)
        {
            //						kfree_skb(skb, FREE_WRITE);
            return 0;
        }
        // if is not broadcast packet, forworad it 
        ip_forward(skb, dev, is_flag);
    }

    if(is_flag)
    {
        skb = ip_defrag(iph, skb, dev);
        if(skb == NULL)
            return 0;
        skb->dev = dev;
        iph = skb->h.iph;
    }

    skb->ip_hdr = iph;
    skb->h.raw += iph->ihl * 4;

    hash = iph->protocol & (MAX_INET_PROTOS - 1);
    for(ipprot = (struct inet_protocol *)inet_protos[hash]; ipprot != NULL; ipprot = ipprot->next)
    {
        struct sk_buff *skb2;
        if(ipprot->protocol != iph->protocol)
            continue;
        ipprot->handler(skb2, dev, opts_p?&opt:NULL, iph->daddr, (ntohs(iph->tot_len) - (iph->ihl * 4)), iph->saddr, 0, ipprot);


    }
    return 0;
}

int ip_id_count = 0;

void ip_queue_xmit(struct sock *sk, struct net_device *dev, struct sk_buff *skb, int free)
{
    struct iphdr *iph;
    unsigned char *ptr;
    if(dev == NULL)
    {
        printk("IP : ip_queue_xmit dev = NULL\n");
        return;
    }

    skb->dev = dev;
    skb->when = jiffies;

    ptr = skb->data;
    ptr += dev->hard_header_len;
    iph = (struct iphdr*)ptr;
    skb->ip_hdr = iph;
    iph->tot_len = ntohs(skb->len - dev->hard_header_len);

    if(free != 2)
        iph->id = htons(ip_id_count++);
    else 
        free = 1;

    if(sk == NULL)
        free = 1;

    skb->free = free;

    if(skb->len > dev->mtu + dev->hard_header_len)
    {
        ip_fragment(sk, skb, dev, 0);
        //	kfree_skb(skb, FREE_WRITE);
        return;
    }

    ip_send_checksum(iph);

    if(skb->next != NULL)
    {
        printk("ip_queue_xmit: next != NULL");
        skb_unlink(skb);
    }

    if(!free)
    {
        unsigned long flags;
        sk->packets_out++;
        save_flags(flags);
        cli();
        if(skb->cache_next != NULL)
        {
            skb->cache_next = NULL;

        }

        if(sk->send_head == NULL)
        {
            sk->send_head = skb;
            sk->send_tail = skb;
        }
        else
        {
            sk->send_head->cache_next = skb;
            sk->send_tail = skb;
        }
        restore_flags(flags);
    }
    else
        skb->sk = sk;

    if(dev->flags & IFF_UP)
    {
        if(sk != NULL)
            dev_queue_xmit(skb, dev, sk->priority);
        else
            dev_queue_xmit(skb, dev, SOPRI_NORMAL);
    }
    else
    {
        //				if(free)
        //						kfree_sbk(skb, FREE_WRITE);
        return;
    }
    return;
}


int ip_setsockopt(struct sock *sk, int level, int optname, char *optval, int optlen)
{
    return 0;
}

int ip_getsockopt(struct sock *sk, int level, int optname, char *optval, int *optlen)
{
    return 0;
}

void ip_init()
{
    ip_packet_type.type = htons(ETH_P_IP);
    dev_add_pack(&ip_packet_type);
    register_netdevice_notifier(&ip_rt_notifier);
}
