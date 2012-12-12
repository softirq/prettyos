
extern void arp_init();
extern int arp_find(unsigned char *haddr,unsigned long paddr,struct net_device *dev,unsigned saddr,struct sk_buff *skb);

extern int arp_device_down(struct net_device *dev);
