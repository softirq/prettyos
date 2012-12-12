#include "type.h"
#include "const.h"
#include "traps.h"
//#include "string.h"
#include "sched.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "kernel.h"
#include "lib.h"
#include "stddef.h"

#include "asm-i386/system.h"
#include "linux/net.h"
#include "linux/socket.h"
#include "linux/skbuff.h"
#include "linux/timer.h"
#include "linux/tcp.h"
#include "ip.h"
#include "arp.h"
#include "tcp.h"
#include "udp.h"
#include "raw.h"

#include "sock.h"
static struct proto_ops inet_protocol =
{
	AF_INET,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

static int sk_inuse(struct proto *prot, int num)
{
		struct sock *sk;
		for(sk = prot->sock_array[num & (SOCK_ARRAY_SIZE - 1)]; sk != NULL; sk = sk->next)
		{
				if(sk->num == num)
						return 1;
		}
		return 0;
}

unsigned short get_new_socknum(struct proto *prot, unsigned short base)
{
		static int start = 0;
		int i,j;
		int best = 0;
		int size = 32767;
		struct sock *sk;

		if(base == 0)
				base = PROT_SOCK + 1 + (start % PROT_SOCK);
		if(base <= PROT_SOCK)
				base += PROT_SOCK + (start % PROT_SOCK);

		for(i = 0; i < SOCK_ARRAY_SIZE; i++)
		{
				j = 0;
				sk = prot->sock_array[(i + base + 1) & (SOCK_ARRAY_SIZE - 1)];
				while(sk != NULL)
				{
						sk = sk->next;
						j++;
				}
				if(j == 0)
				{
						start = (i + 1 + start)%PROT_SOCK;
						return (i + base + 1);
				}
				if(j < size)
				{
						best = i;
						size = j;
				}
		}
		while(sk_inuse(prot, base + best + 1))
		{
				best += SOCK_ARRAY_SIZE;
		}
		return (base + best + 1);
}

void put_sock(unsigned short num, struct sock *sk)
{
		struct sock *sk1;

		unsigned long flags;
		sk->num = num;
		sk->next = NULL;
		num = num & (SOCK_ARRAY_SIZE - 1);

		save_flags(flags);
		cli();

		sk->prot->inuse = 1;
		if(sk->prot->sock_array[num] == NULL)
		{
				sk->prot->sock_array[num] = sk;
				restore_flags(flags);
				return;
		}
		restore_flags(flags);
		sk1 = sk->prot->sock_array[num & (SOCK_ARRAY_SIZE -1)];
		while(sk1 != NULL)
				sk1 = sk1->next; 
		sk1->next = sk;
		sk->next = NULL;
		return;
}

static void remove_sock(struct sock *sk)
{
		struct sock *sk1;
		unsigned long flags;
		if(!sk->prot)
		{
				printk("remove_sock:sk->prot = NULL\n");
				return;
		}
		save_flags(flags);
		cli();
		sk1 = sk->prot->sock_array[sk1->num & (SOCK_ARRAY_SIZE -1)];
		if(sk1 == sk)
		{
				sk1->prot->inuse -= 1;
				sk->prot->sock_array[sk1->num & (SOCK_ARRAY_SIZE -1)] = sk1->next;
				restore_flags(flags);
				return;
		}

		while(sk1 && sk1->next != sk)
		{
				sk1 = sk1->next;
		}
		if(sk1)
		{
				sk->prot -= 1;
				sk1->next = sk->next;
				restore_flags(flags);
				sk->next = NULL;
		}
		restore_flags(flags);
}



struct sock *get_sock(struct proto *prot, unsigned short num,
				unsigned long raddr, 
				unsigned short rnum, unsigned long laddr)
{
		struct sock *sk;
		return sk;
}

void inet_proto_init(struct net_proto *proto)
{
		int i;
		struct inet_protocol *p;

		(void)sock_register(inet_protocol.family,&inet_protocol);

		for(i = 0;i < SOCK_ARRAY_SIZE;i++)
		{
				tcp_prot.sock_array[i] = NULL;
				udp_prot.sock_array[i] = NULL;
				raw_prot.sock_array[i] = NULL;

		}
		tcp_prot.inuse = 0;
		tcp_prot.highestinuse = 0; 

		udp_prot.inuse = 0;
		udp_prot.highestinuse = 0; 

		raw_prot.inuse = 0;
		raw_prot.highestinuse = 0; 

		printk("IP Protocols:\n");

		for(p = inet_protocol_base ; p!=NULL; )
		{

				struct inet_protocol *tmp = p->next;
				inet_add_protocol(p);
				printk("%s%s",p->name,tmp?",":"\n");

		}

		arp_init();

		ip_init();

		return;
}
