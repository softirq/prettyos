#ifndef     _NET_DEVICE__H_
#define     _NET_DEVICE__H_

#include "linux/skbuff.h"
#include "linux/notifier.h"

#define 	MAX_ADDR_LEN   	7
#define 	DEV_NUMBUFFS 	3
#define 	MAX_HEADER 		18

extern volatile char in_bh;

struct net_device
{
    unsigned short family;
    volatile unsigned char start; 
    volatile unsigned char tbusy; // transmitter busy
    unsigned short mtu;
    char *name;
    int (*init)(struct net_device *dev);
    int (*hard_header)(unsigned char *buffer,struct net_device *dev,unsigned short type,void *daddr, void *saddr,unsigned int len,struct sk_buff *skb);
    unsigned int (*type_trans)(struct sk_buff *skb,struct net_device *dev);
    int (*rebuild_header)(void *buff,struct net_device *dev,unsigned long dst,struct sk_buff *skb);
    int (*hard_start_xmit)(struct sk_buff *skb,struct net_device *dev);
    struct net_device *next;

    unsigned char addr_len;
    unsigned short  type;   	//interface hardward type
    unsigned long pa_addr; //protocol address
    unsigned long pa_broadcast;
    unsigned long pa_mask;
    unsigned short pa_alen;
    void *daddr;
    void *saddr;
    unsigned short hard_header_len;

    int (*open)(struct net_device *dev);
    int (*stop)(struct net_device *dev);

    //hardware buffer queue
    struct sk_buff_head 	buffs[DEV_NUMBUFFS];

    unsigned char flags;

    unsigned char broadcast[MAX_ADDR_LEN];
    unsigned char dev_addr[MAX_ADDR_LEN];
};

struct packet_type 
{
    unsigned short 	type;
    struct net_device 	*dev;
    int (*func)(struct sk_buff *skb,struct net_device *dev,struct packet_type *pt);
    void 	*data;
    struct packet_type *next;
};

extern  struct net_device *dev_base;

extern void dev_add_pack(struct packet_type *pt);
extern void dev_init();
extern void ether_setup(struct net_device *dev); 
extern int  register_netdev(struct net_device *dev);
extern void unregister_netdev(struct net_device *dev);

//extern int  register_netdevice_notifier(struct notifier_block *nb);
//extern int  unregister_netdevice_notifier(struct notifier_block *nb);
//extern struct net_device *dev_base;

extern 	void 	dev_queue_xmit(struct sk_buff *skb, struct net_device *dev, int pri);
extern 	int 	ip_chk_addr(unsigned long addr);
extern  int     register_netdevice_notifier(struct notifier_block *nb);
extern  int     unregister_netdevice_notifier(struct notifier_block *nb);

#define  	IS_MYADDR  		1
#define 	IS_LOOPBACK 	2
#define  	IS_BROADCAST 	3
#define 	IS_MULTICAST 	4

#endif
