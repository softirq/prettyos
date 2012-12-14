#ifndef     _INET_SOCK_H_
#define     _INET_SOCK_H_

#define  	SOCK_ARRAY_SIZE 	256
#include "asm-i386/param.h"

#include "linux/ip.h"
#include "linux/in.h"
#include "protocol.h"

struct proto 
{
    unsigned short        	max_header; 
    unsigned long         	retransmits;
    struct 	sock* 			sock_array[SOCK_ARRAY_SIZE];
    char 					name[30];
    int 					inuse, highestinuse;

    struct sk_buff* (*wmalloc)(struct sock *sk, unsigned long size, int force, int priority);
    struct sk_buff* (*rmalloc)(struct sock *sk, unsigned long size, int force, int priority);

    void (*wfree) (struct sock *sk, struct sk_buff *skb, unsigned long size);
    void (*rfree) (struct sock *sk, struct sk_buff *skb, unsigned long size);

    unsigned long (*rspace)(struct sock *sk);
    unsigned long (*wspace)(struct sock *sk);

    void (*close)(struct sock *sk, int timeout);
    int (*read)(struct sock *sk, unsigned char *to, int len, int nonblock, unsigned int flags);
    int (*write)(struct sock *sk, unsigned char *to, int len, int noblock, unsigned int flags);
    int (*sendto)(struct sock *sk, unsigned char *from, int len, int noblock, unsigned int flags,\
            struct sockaddr_in *usin, int addr_len);
    int (*recvfrom)(struct sock *sk, unsigned char *from, int len, int nonblock, unsigned int flags,\
            struct sockaddr_in *usin, int addr_len);
    int (*build_header)(struct sk_buff *skb, unsigned long saddr, unsigned long daddr, struct net_device **dev, \
            int type, struct options *opt, int len, int tos, int ttl);
    int (*connect)(struct sock *sk, struct sockaddr_in *usin, int flags);
    void (*queue_xmit)(struct sock *sk, struct net_device *dev, struct sk_buff *skb, int free);
    void (*retransmit)(struct sock *sk, int all);
    void (*write_wakeup)(struct sock *sk);
    void (*read_wakeup)(struct sock *sk);
    int (*rcv)(struct sk_buff *skb, struct net_device *dev, struct options *opt, unsigned long daddr, \
            unsigned short len,	unsigned long saddr, int redo, struct inet_protocol *protocol);
    int (*ioctl)(struct sock *sk, int cmd, unsigned long arg);
    int (*init)(struct sock *sk);
    void (*shutdown)(struct sock *sk, int how);
    void (*setsockopt)(struct sock *sk, int level, int optname, char *optval, int optlen);
    int (*getsockopt)(struct sock *sk, int level, int optname, char *optval, int *optlen);

};

struct sock
{
    struct sock  	*next;
    struct sock 	*prev;
    unsigned short 	mtu;
    unsigned long 	saddr;
    unsigned long 	daddr;
    unsigned int 	priority;

    struct proto 	*prot;

    struct sk_buff 	 	volatile 	*send_head;
    struct sk_buff 		volatile 	*send_tail;

    volatile unsigned long    wmem_alloc;
    volatile unsigned long    rmem_alloc;

    volatile unsigned long packets_out;
    volatile unsigned char state;
    volatile short 	err;
    volatile char 	inuse;
    volatile char	dead; //dead = 1, sock is ready for free
    volatile char  	ack_backlog;
    volatile char  	max_ack_backlog;
    volatile char  	zapped;
    volatile char  	ack_timed;
    volatile char  	keepopen;
    volatile char  	done;

    unsigned char localroute;

    long 	retransmits;

    unsigned long   window_clamp;  
    unsigned long 	write_seq;
    unsigned long 	sent_seq;
    unsigned long 	acked_seq;
    unsigned long 	copied_seq;
    unsigned long 	window_seq;
    unsigned long 	fin_seq;;
    unsigned long 	rcv_ack_seq;;

    struct timer_list 		retransmit_timer;
    struct timer_list 		timer;

    volatile unsigned short 	cong_window;
    volatile unsigned short 	cong_count;
    volatile unsigned short 	ssthresh;
    volatile unsigned short 	mss;
    volatile unsigned short 	max_window;
    volatile unsigned long 		rto;
    volatile unsigned long 		rtt;
    volatile unsigned short 	backoff;
    volatile unsigned short 	shutdown;
    volatile unsigned short 	window;
    volatile unsigned short   	user_mss;  /* mss requested by user in ioctl */     

    unsigned short 		bytes_rcv;
    unsigned short 		num;
    int 	ip_xmit_timeout; 		// why the timeout is running
    int 	ip_tos;
    int 	ip_ttl;
    int 	timeout;

    struct 	tcphdr         dummy_th; 

    struct options *opt;
    struct sk_buff 		*partial;
    struct timer_list 	partial_timer;
    //packet from  backlog
    struct sk_buff_head 	receive_queue;
    // wait for send
    struct sk_buff_head 	write_queue;
    struct sk_buff_head 	back_log;
    struct 	wait_queue 		**sleep;

    struct socket *socket;

    void (*state_change)(struct sock *sk);
    void (*write_space)(struct sock *sk);
    void (*data_ready)(struct sock *sk,int bytes);
    void (*err_report)(struct sock *sk);

};

extern  	void release_sock(struct sock *sk);
extern struct sock      *get_sock(struct proto *, unsigned short,
        unsigned long, unsigned short,
        unsigned long);

#define 	TIME_WRITE 		0x01
#define 	TIME_CLOSE 		0x02
#define 	TIME_KEEPOPEN 	0x03

#define 	TIME_PROBE 		0x06

#define 	PROT_SOCK 		1024
#define 	RCV_SHUTDOWN 	0x01
#define 	SEND_SHUTDOWN 	0x02
#define 	SHUTDOWN_MASK 	0x03

void net_timer (unsigned long);

#endif
