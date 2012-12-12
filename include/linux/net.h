#define NPROTO          16

#define SO_WAITDATA (1<<17)     /* wait data to read        */
#define SO_NOSPACE  (1<<18)     /* no space to write        */

struct net_proto 
{              
		char *name;             /* Protocol name */           
		void (*init_func)(struct net_proto *);  /* Bootstrap */        
};


struct proto_ops
{
		int family;
		int (*create)();
		int (*connect)();
		int (*bind)();
		int (*read)();
		int (*write)();
};


struct socket
{
		long 	flags;
};

extern int init_sock(void);
extern int sock_register(int family, struct proto_ops *ops); 
