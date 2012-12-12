#define MAX_INET_PROTOS 	32

struct inet_protocol 
{
		int (*handler)(struct sk_buff *skb, struct net_device *dev,struct options *opt, unsigned long daddr, \
						unsigned short len, unsigned long saddr,	int redo, struct inet_protocol *protocol);
		struct inet_protocol *next;
		unsigned char protocol;
		void 	*data;
		char 	*name;
};


extern struct inet_protocol *inet_protocol_base;
extern struct inet_protocol *inet_protos[MAX_INET_PROTOS];


extern void inet_add_protocol(struct inet_protocol *prot);
