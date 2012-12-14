#ifndef     _SKBUFF_H_
#define     _SKBUFF_H_

#define FREE_READ 	0
#define FREE_WRITE  1

struct sk_buff_head
{
    struct sk_buff *next;
    struct sk_buff *prev;
};

struct sk_buff
{
    struct sk_buff *next;
    struct sk_buff *prev;
    unsigned char pkt_type;  //pakcet type host/broadcat/multicast/forward

#define PACKET_HOST 		0
#define PACKET_BROADCAST  	1
#define PACKET_MULTICAST 	2
#define PACKET_FORWARD 		3

    unsigned long 	len;
    struct net_device 	*dev;
    volatile char 	free;	
    volatile char 	arp;	

    unsigned short 		users;
    unsigned long 		saddr;
    unsigned long 		daddr;
    unsigned long  		raddr; /* next hop addr */

    volatile char 	used;


    struct sk_buff        *mem_addr; 
    union
    {
        struct tcphdr 	*th;
        struct udphdr 	*uh;
        struct ethhdr 	*eth;
        struct iphdr 	*iph;
        unsigned long  	seq;
        unsigned char 	*raw;

    }h;

    volatile unsigned long when;   //when send packet time

    struct 	iphdr 	*ip_hdr;
    unsigned long         mem_len;   
    struct sock 	*sk;
    unsigned char 	localroute;

    struct sk_buff *cache_next;

    unsigned char data[0]; 	//point to data
};

static inline struct sk_buff *skb_peek(struct sk_buff_head *_list)
{
    struct sk_buff *list = (struct sk_buff *)_list;
    return (list->next != list)?list->next:NULL;
}

extern struct sk_buff* skb_dequeue(struct sk_buff_head* head);
extern void dev_kfree_skb(struct sk_buff *skb,int mode);
extern struct sk_buff* alloc_skb(unsigned int size,int priority); 
extern void skb_queue_tail(struct sk_buff_head *skh,struct sk_buff *skb);
extern void skb_queue_head(struct sk_buff_head *skh,struct sk_buff *skb);
extern void skb_unlink(struct sk_buff *skb);

#endif
