
struct rtable
{
		struct rtable 	*rt_next;
		unsigned long 	rt_dst;
		unsigned long 	rt_mask;
		unsigned long 	rt_gateway;
		unsigned char 	rt_flags;
		unsigned char 	rt_metric;
		short 			ft_refcnt;
		unsigned long 	rt_use;
		unsigned short 	rt_mss;
		unsigned long 	rt_window;
		struct net_device 	*rt_dev;
};

extern void 		    ip_rt_flush(struct net_device *dev);
extern struct rtable    *ip_rt_local(unsigned long daddr, struct options *opt, unsigned long *src_addr);
extern struct rtable    *ip_rt_route(unsigned long daddr, struct options *opt, unsigned long *src_addr);

