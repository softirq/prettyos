#include "type.h"
#include "stddef.h"
#include "errno.h"
#include "lib.h"
#include "printf.h"
#include "traps.h"
#include "fcntl.h"

#include "asm-i386/system.h"
#include "asm-i386/byteorder.h"
#include "linux/sched.h"
#include "linux/wait.h"
#include "linux/mm.h"

#include "linux/netdevice.h"
#ifndef  _SKBUFF_HEADER_H
#define  _SKBUFF_HEADER_H
#include "linux/skbuff.h"
#endif
#include "linux/net.h"
#include "linux/tcp.h"
#include "linux/route.h"
#include "linux/icmp.h"
#include "linux/if.h"
#include "linux/timer.h"

#include "route.h"
#include "sock.h"
#include "tcp.h"
#include "ip.h"

static void tcp_close(struct sock *sk, int timeout);

struct  proto tcp_prot = {
    128,
    0,
    {NULL,},
    "TCP",
    0,
    0
};

static struct wait_queue *master_select_wakeup;

static inline int min(unsigned int a, unsigned int b)
{
    return (a > b?b:a);
}

static inline int max(unsigned int a, unsigned int b)
{
    return (a > b?a:b);
}

static inline void tcp_set_state(struct sock *sk, int state)
{
    if((sk->state == TCP_SYN_RECV) && (state == TCP_ESTABLISHED))
    {
        wake_up_interruptible(&master_select_wakeup);

    }
    sk->state = state;
    return;
}

int tcp_select_window(struct sock *sk)
{
    return 0;
}

static struct sk_buff* tcp_find_established(struct sock *sk)
{
    struct sk_buff *p = skb_peek(&sk->receive_queue);
    if(p == NULL)
        return NULL;
    do
    {
        if(p->sk->state == TCP_ESTABLISHED || p->sk->state == TCP_FIN_WAIT2)
            return p;
        p = p->next;
    }
    while(p != (struct sk_buff*)&sk->receive_queue); 
    return NULL;
}

static int select_wait(struct wait_queue **wq,struct wait_queue *wait)
{
    wait->task = current;
    wait->next = NULL;
    add_wait_queue(wq, wait);
    return 0;
}
static int tcp_listen_select(struct sock *sk, int sel_type, struct wait_queue *wait)
{
    if(sel_type == SEL_IN)
    {
        int retval = 0;
        sk->inuse = 1;
        retval = (tcp_find_established(sk) != NULL);
        release_sock(sk);
        if(!retval)
            select_wait(&master_select_wakeup,wait);
        return retval;
    }
    return 0;
}

static struct sk_buff *tcp_dequeue_established(struct sock *sk)
{
    struct sk_buff *skb;
    unsigned long flags;
    save_flags(flags);
    cli();

    skb = tcp_find_established(sk);
    if(skb != NULL)
        skb_unlink(skb);
    restore_flags(flags);
    return skb;
}

static int tcp_select(struct sock *sk, int sel_type, struct wait_queue *wait)
{
    if(sk->state == TCP_LISTEN)
        return tcp_listen_select(sk, sel_type, wait);

    switch(sel_type)
    {
        case SEL_IN:
            if(sk->err)
                return 1;
            if(sk->state == TCP_SYN_SENT || sk->state == TCP_SYN_RECV)
                break;
            if(sk->shutdown & RCV_SHUTDOWN)
                return 1;
            if(sk->acked_seq == sk->copied_seq)
                break;
            else 
                return 1;
            break;

        case SEL_OUT:
            if(sk->shutdown & SEND_SHUTDOWN)
                return 0;
            if(sk->state == TCP_SYN_SENT || sk->state == TCP_SYN_SENT)
                break;
            break;
        case SEL_EX:
            if(sk->err)
                return 1;
            break;
    }
    select_wait(sk->sleep, wait);
    return 0;
}

int tcp_ioctl(struct sock *sk, int cmd, unsigned long arg)
{
    //	int err;
    switch(cmd)
    {
        default:
            break;
    }
    return 0;
}

static struct sock *tcp_accept(struct sock *sk, int flags)
{
    struct sock *newsk;
    struct sk_buff *skb;

    if(sk->state != TCP_LISTEN)
    {
        sk->err = EINVAL;
        return NULL;
    }
    cli();
    sk->inuse = 1;
    while((skb = tcp_dequeue_established(sk)) == NULL)
    {
        if(flags & O_NONBLOCK)
        {
            sti();
            release_sock(sk);
            sk->err = EAGAIN;
            return NULL;
        }
        release_sock(sk);
        interruptible_sleep_on(sk->sleep);
        /*			if(current->signal & ~current->blocked)
                    {
                    sti();
                    sk->err = ERESTARTSYS;
                    return NULL;
                    }
                    */
        sk->inuse = 1;
    }
    sti();
    newsk = skb->sk;
    //		kfree_skb(skb, FREE_READ);
    sk->ack_backlog--;
    release_sock(sk);
    return newsk;
}

static void tcp_close_pending(struct sock *sk)
{
    struct sk_buff *skb;
    while((skb = skb_dequeue(&sk->receive_queue)) != NULL)
    {
        skb->sk->dead = 1;
        tcp_close(skb->sk, 0);
        //				kfree_skb(skb, FREE_READ);
    }
    return;
}

void tcp_send_checksum(struct tcphdr *th, unsigned long saddr, \
        unsigned long daddr, int len, struct sock *sk)   
{
    return;
}

static void reset_xmit_timer(struct sock *sk, int why, unsigned long when)
{
    del_timer(&sk->retransmit_timer);
    sk->ip_xmit_timeout = why;
    if((int)when < 0) 
    {    
        when=3;
        printk("Error: Negative timer in xmit_timer\n");
    }    
    sk->retransmit_timer.expires=when;
    add_timer(&sk->retransmit_timer);

}

static void tcp_send_skb(struct sock *sk, struct sk_buff *skb)
{
    int size;
    struct tcphdr *th = skb->h.th;

    size = skb->len - ((unsigned char  *)th - skb->data);
    if(size < sizeof(struct tcphdr) || size > skb->len)
    {
        printk("tcp_send_skb:bad skb\n");
        //			kfree_skb(skb, FREE_WRITE);
        return;
    }

    if(size == sizeof(struct tcphdr))
    {
        printk("tcp_send_skb:");
        //			kfree_skb(skb, FREE_WRITE);
        return;
    }

    skb->h.seq = ntohl(th->seq) + size - (th->len >> 2);
    if(after(skb->h.seq, sk->window_seq) || (sk->retransmits && sk->ip_xmit_timeout == TIME_WRITE) || \
            sk->packets_out >= sk->cong_window)
    {
        th->check = 0;
        if(skb->next != NULL)
        {
            printk("tcp_send_patial:next != NULL\n");
            skb_unlink(skb);
        }
        skb_queue_tail(&sk->write_queue, skb);
        if(before(sk->window_seq,sk->write_queue.next->h.seq) && sk->send_head == NULL && sk->ack_backlog ==0)
            reset_xmit_timer(sk, TIME_PROBE, sk->rto);
    }
    else
    {
        th->ack_seq = ntohl(sk->acked_seq);
        th->window = ntohs(tcp_select_window(sk));

        tcp_send_checksum(th, sk->saddr, sk->daddr, size, sk);
        sk->sent_seq = sk->write_seq;

        sk->prot->queue_xmit(sk, skb->dev, skb, 0);
        reset_xmit_timer(sk, TIME_WRITE, sk->rto);
    }
    return;
}

static void tcp_write_timeout(struct sock *sk)
{
    return;
}


void tcp_do_retransmit(struct sock *sk, int all)
{
    struct sk_buff *skb;
    struct proto *prot;
    struct net_device *dev;
    int ct = 0;

    prot = sk->prot;
    skb = sk->send_head;
    while(skb != NULL)
    {
        struct tcphdr *th;
        struct iphdr  *iph;
        int size;

        dev = skb->dev;
        skb->when = jiffies;

        iph = (struct iphdr*)(skb->data + dev->hard_header_len);
        th = (struct tcphdr*)((char *)iph + (iph->ihl << 2));
        size = skb->len - ((unsigned char *)th - skb->data);
        iph->id = htons(ip_id_count++);
        ip_send_checksum(iph);

        th->ack_seq = ntohl(sk->acked_seq);
        th->window = ntohs(tcp_select_window(sk));
        tcp_send_checksum(th, sk->saddr, sk->daddr, size, sk);

        if(dev->flags & IFF_UP)
        {
            if(sk)
            {
                skb_unlink(skb);
                dev_queue_xmit(skb, dev, sk->priority);
            }
        }
        ct++;
        sk->prot->retransmits++;
        if(!all)
            break;

        if(ct > sk->cong_window)
            break;
        skb = skb->cache_next;
    }
    return;
}

void tcp_retransmit_time(struct sock *sk, int all)
{

    tcp_do_retransmit(sk, all);
    sk->retransmits++;
    sk->backoff++;
    sk->rto = min(sk->rto << 1,120 * HZ);
    reset_xmit_timer(sk, TIME_WRITE, sk->rto);
}

static void tcp_retransmit(struct sock *sk, int all)
{
    if(all)
    {
        tcp_retransmit_time(sk, all);
        return;
    }
    sk->ssthresh = sk->cong_window >> 1;
    sk->cong_count = 0;
    sk->cong_window = 1;
    tcp_retransmit_time(sk, all);
}

static void retransmit_timer(unsigned long data)
{
    struct sock *sk = (struct sock*)data;
    int reason = sk->ip_xmit_timeout;
    cli();
    if(sk->inuse || in_bh)
    {
        sk->retransmit_timer.expires = HZ;
        add_timer(&sk->retransmit_timer);
        sti();
        return;
    }

    sk->inuse = 1;
    sti();

    if(sk->ack_backlog && !sk->zapped) // if some packets need to ack and not reset 
    {
        sk->prot->read_wakeup(sk);
        if(!sk->dead)
            sk->data_ready(sk,0);
    }
    switch(reason)
    {
        //window probing 
        case TIME_PROBE:
            tcp_send_probe(sk);
            tcp_write_timeout(sk);
            break;
            // retransmitting
        case TIME_WRITE:
            {
                struct sk_buff *skb;
                unsigned long flags;
                save_flags(flags);
                cli();
                skb = sk->send_head;
                if(!skb)
                {
                    restore_flags(flags);
                }
                else
                {
                    if(jiffies < skb->when + sk->rto)
                    {
                        reset_xmit_timer(sk, TIME_WRITE, skb->when + sk->rto - jiffies);
                        restore_flags(flags);
                        return;
                    }
                    restore_flags(flags);
                    sk->prot->retransmit(sk, 0);

                }
                break;
            }
            //send keepalive
        case TIME_KEEPOPEN:
            reset_xmit_timer(sk, TIME_KEEPOPEN, TCP_TIMEOUT_LEN);
            if(sk->prot->write_wakeup)
                sk->prot->write_wakeup(sk);
            sk->retransmits++;
            tcp_write_timeout(sk);
            break;
        default:
            break;

    }
    release_sock(sk);
    return;
}

void tcp_err(int err, unsigned char *header, unsigned long saddr, unsigned long daddr, struct inet_protocol *protocol)
{
    struct tcphdr   *th;
    struct sock     *sk;
    struct iphdr    *iph;

    iph = (struct iphdr*)header;
    header += iph->ihl >> 2;

    th = (struct tcphdr*)header;

    sk = get_sock(&tcp_prot, th->source, daddr, th->dest, saddr);
    if(sk == NULL)
        return;

    if(err < 0) //serious error
    {
        sk->err = err;
        sk->err_report(sk);
        return;
    }

    if((err & 0xff00) == (ICMP_SOURCE_QUENCH << 8))
    {
        if(sk->cong_window > 4)
            sk->cong_window--;
        return;
    }

    if(sk->state == TCP_SYN_SENT)
    {
        tcp_set_state(sk, TCP_CLOSE);
        sk->err_report(sk);
    }
    return;
}

struct sk_buff* tcp_dequeue_partial(struct sock *sk)
{
    struct sk_buff *skb;
    unsigned long flags;

    save_flags(flags);
    cli();
    skb = sk->partial;
    if(skb)
    {
        sk->partial = NULL;
        del_timer(&sk->partial_timer);
    }
    restore_flags(flags);
    return skb;
}

static void tcp_send_partial(struct sock *sk)
{
    struct sk_buff *skb;
    if(sk == NULL)
        return;
    while((skb = tcp_dequeue_partial(sk) != NULL))
    {
        tcp_send_skb(sk, skb);
    }
}

void tcp_enqueue_partial(struct sk_buff *skb, struct sock *sk)
{
    struct sk_buff *tmp;
    unsigned long flags;
    save_flags(flags);
    cli();

    tmp = sk->partial;
    if(tmp)
        del_timer(&sk->partial_timer);
    sk->partial = skb;
    init_timer(&sk->partial_timer);
    sk->partial_timer.expires = HZ;
    sk->partial_timer.function = (void (*)(unsigned long))tcp_send_partial;
    sk->partial_timer.data = (unsigned long)sk;
    add_timer(&sk->partial_timer);
    restore_flags(flags);

    if(tmp)
        tcp_send_skb(sk,tmp);
}

static void tcp_send_ack(unsigned long sequeue, unsigned long ack, struct sock *sk, struct tcphdr *th, 
        unsigned long daddr)
{
    struct sk_buff *skb;
    struct tcphdr *th2;
    struct net_device *dev;
    int tmp;

    if(sk->zapped)
        return;

    skb = sk->prot->wmalloc(sk, MAX_ACK_SIZE, 1, GFP_ATOMIC);
    if(skb == NULL)
    {
        sk->ack_backlog++;
        if(sk->ip_xmit_timeout != TIME_WRITE && tcp_connected(sk->state))
            reset_xmit_timer(sk, TIME_WRITE, HZ);
    }

    skb->len = sizeof(struct tcphdr);
    skb->sk = sk;
    skb->localroute = sk->localroute;
    th2 = (struct tcphdr*)skb->data;

    tmp = sk->prot->build_header(skb, sk->saddr, daddr, &dev, IPPROTO_TCP, sk->opt, MAX_ACK_SIZE, sk->ip_tos, sk->ip_ttl);
    if(tmp < 0)
    {
        skb->free = 1;
        sk->prot->wfree(sk, skb->mem_addr, skb->mem_len);
    }
    skb->len += tmp;
    th2 = (struct tcphdr*)((char *)th2 + tmp);
    memncpy(th2, th, sizeof(struct tcphdr));

    th2->dest = th->source;
    th2->source = th->dest;
    th2->seq = ntohl(sequeue);
    th2->ack = 1;
    sk->window = tcp_select_window(sk);
    th2->window = ntohs(sk->window);
    th2->len = 0;
    th2->reserve = 0;
    th2->urg = 0;
    th2->rst = 0;
    th2->syn = 0;
    th2->psh = 0;
    th2->fin = 0;

    if(ack == sk->acked_seq)
    {
        sk->ack_backlog = 0;
        sk->bytes_rcv = 0;
        sk->ack_timed = 0;
        if(sk->send_head == NULL && skb_peek(&sk->write_queue) == NULL && sk->ip_xmit_timeout == TIME_WRITE)
        {
            if(sk->keepopen)
                reset_xmit_timer(sk, TIME_KEEPOPEN, TCP_TIMEOUT_LEN);
            else
                delete_timer(sk);
        }
    }

    th2->ack_seq = ntohl(ack);
    th2->len = sizeof(struct tcphdr)/4;
    tcp_send_checksum(th2, sk->saddr, daddr, sizeof(struct tcphdr), sk);
    sk->prot->queue_xmit(sk, dev, skb, 1);
    return;
}

static int tcp_write(struct sock *sk, unsigned char *from, int len, int nonblock, unsigned long flags)
{
    int tmp;
    int copy;
    unsigned char *buff;

    struct sk_buff *skb;
    struct sk_buff *send_tmp;
    struct proto *prot;
    struct net_device *dev = NULL;

    sk->inuse = 1;
    prot = sk->prot;

    while(len > 0)
    {
        if(sk->err)
        {
            release_sock(sk);
            tmp = -sk->err;
            sk->err = 0;
            return tmp;
        }
        if(sk->shutdown & SEND_SHUTDOWN)
        {
            release_sock(sk);
            sk->err = EPIPE;
            sk->err = 0;
            return (-EPIPE);
        }
        while(sk->state != TCP_ESTABLISHED && sk->state != TCP_CLOSE_WAIT)
        {
            if(sk->err)
            {
                release_sock(sk);
                tmp = -sk->err;
                sk->err = 0;
                return tmp;
            }
            if(sk->state != TCP_SYN_SENT && sk->state != TCP_SYN_RECV)
            {
                release_sock(sk);
                if(sk->err)
                {
                    release_sock(sk);
                    tmp = -sk->err;
                    sk->err = 0;
                    return tmp;
                }
                if(sk->keepopen)
                {
                    //						send_sig(SIGPIPE, current, 0);
                }

            }
            if(nonblock)
            {
                release_sock(sk);
                return (-EAGAIN);
            }

            release_sock(sk);
            cli();

            if(sk->state != TCP_ESTABLISHED && sk->state != TCP_CLOSE_WAIT && sk->err == 0)
            {
                interruptible_sleep_on(sk->sleep);
                if(current->signal  & ~current->blocked)
                {
                    sti();
                    return -ERESTARTSYS; 
                }
            }
            sk->inuse = 1;
            sti();

        }

        if((skb = tcp_dequeue_partial(sk)) != NULL)
        {
            int hdrlen;
            hdrlen = (unsigned long)skb->h.th - (unsigned long)skb->data + sizeof(struct tcphdr);
            if(!(flags & MSG_OOB))
            {
                copy = min(sk->mss - (skb->len - hdrlen), len);
                if(copy <= 0)
                {
                    printk("tcp write : bug\n");
                    copy = 0;
                }

                memncpy(skb->data + skb->len, from, copy);
                skb->len += copy;
                from += copy;
                len -= copy;
                sk->write_seq += copy;
            }

            if((skb->len - hdrlen) >= sk->mss || (flags & MSG_OOB) || !sk->packets_out )
            {
                tcp_send_skb(sk, skb);
            }
            else
                tcp_enqueue_partial(skb, sk);
            continue;
        }

        copy = sk->window_seq - sk->write_seq;
        if(copy <= 0 || copy < (sk->max_window >> 1) || copy > sk->mss)
            copy = sk->mss;
        if(copy > len)
            copy = len;

        send_tmp = NULL;
        if(copy < sk->mss && !(flags & MSG_OOB))
        {
            release_sock(sk);
            skb = prot->wmalloc(sk, sk->mtu + 128 + prot->max_header, 0, GFP_KERNEL);
            sk->inuse = 1;
            send_tmp = skb;
        }
        else
        {
            release_sock(sk);
            skb = prot->wmalloc(sk, copy + prot->max_header, 0 ,GFP_KERNEL);
            sk->inuse = 1;
        }

        if(skb == NULL)
        {
            //			sk->socket->flags |= SO_NOSPACE;
            if(nonblock)
            {
                release_sock(sk);
                return (-EAGAIN);
            }

            release_sock(sk);
            cli();
            interruptible_sleep_on(sk->sleep);
            if(current->signal  & ~current->blocked)
            {
                sti();
                return -ERESTARTSYS; 
            }
            sk->inuse = 1;
            sti();
            continue;
        }
        skb->len = 0;
        skb->sk = sk;
        skb->free = 0;
        skb->localroute = sk->localroute | (flags & MSG_DONTROUTE);

        buff = skb->data;
        tmp = prot->build_header(skb, sk->saddr, sk->daddr,&dev, IPPROTO_TCP, sk->opt, skb->mem_len, sk->ip_tos, sk->ip_ttl);
        if(tmp < 0)
        {
            prot->wfree(sk, skb->mem_addr, skb->mem_len);
            release_sock(sk);
            return tmp;
        }

        skb->len += tmp;
        skb->dev = dev;
        buff += tmp;
        skb->h.th = (struct tcphdr*)buff;
        tmp = tcp_build_header((struct tcphdr*)buff, sk, len - copy);
        if(tmp < 0)
        {
            prot->wfree(sk, skb->mem_addr, skb->mem_len);
            release_sock(sk);
            return tmp;
        }

        skb->len += tmp;
        memncpy(buff+tmp, from ,copy);
        from += copy;
        len -= copy;
        skb->len += copy;
        skb->free = 0;
        sk->write_seq += copy;
        if(send_tmp != NULL && sk->packets_out)
        {
            tcp_enqueue_partial(send_tmp, sk);
            continue;
        }
        tcp_send_skb(sk, skb);
    }
    sk->err = 0;
    if(sk->partial && (!sk->packets_out))
    {
        tcp_send_patial(sk);
    }
    release_sock(sk);
    return 0;
}

static int tcp_sendto(struct sock *sk, unsigned char *from, int len, int nonblock, unsigned long flags, struct sockaddr_in *addr, int addr_len)
{
    if(flags & ~(MSG_OOB | MSG_DONTROUTE))
        return -EINVAL;
    if(sk->state == TCP_CLOSE)
        return -ENOTCONN;
    if(addr_len < sizeof(*addr))
        return -EINVAL;
    if(addr->sin_family && (addr->sin_family != AF_INET))
        return -EINVAL;
    if(addr->sin_addr.s_addr != sk->daddr)
        return -EISCONN;
    return tcp_write(sk, from, len, nonblock, flags);
}

static void tcp_read_wakeup(struct sock *sk)
{
    int tmp;
    struct net_device *dev;
    struct tcphdr *th;
    struct sk_buff *skb;

    if(!sk->ack_backlog)
        return;
    skb = sk->prot->wmalloc(sk, MAX_ACK_SIZE, 1, GFP_ATOMIC);
    if(skb == NULL)
    {
        reset_xmit_timer(sk, TIME_WRITE, HZ);
        return;
    }

    skb->len = sizeof(struct tcphdr);
    skb->sk = sk;
    skb->localroute = sk->localroute;

    tmp = sk->prot->build_header(skb, sk->saddr, sk->daddr, &dev, IPPROTO_TCP, sk->opt, MAX_ACK_SIZE, sk->ip_tos, sk->ip_ttl);
    if(tmp < 0)
    {
        skb->free = 1;
        sk->prot->wfree(sk, skb->mem_addr, skb->mem_len);
        return;
    }

    skb->len += tmp;
    th = (struct tcphdr*)(skb->data + tmp);

    memncpy(th, (char *)&sk->dummy_th, sizeof(*th));

    th->seq = htonl(sk->sent_seq);
    th->ack = 1;
    th->reserve = 0;
    th->rst = 0;
    th->urg = 0;
    th->syn = 0;
    th->psh = 0;
    sk->ack_backlog = 0;
    sk->bytes_rcv = 0;
    sk->window = tcp_select_window(sk);
    th->window = sk->window;
    th->ack_seq = ntol(sk->acked_seq);
    th->len = sizeof(struct tcphdr)/4;
    tcp_send_checksum(th, sk->saddr, sk->daddr, sizeof(*th), sk);
    sk->prot->queue_xmit(sk, dev, skb, 1);
    return;
}

static void tcp_write_wakeup(struct sock *sk)
{
    struct sk_buff  *skb;
    struct tcphdr *th;
    struct net_device *dev = NULL;

    int tmp;

    if(sk->zapped)
        return;
    if(sk->state != TCP_ESTABLISHED && sk->state != TCP_CLOSE_WAIT && sk->state != TCP_FIN_WAIT1 
            && sk->state != TCP_LAST_ACK && sk->state != TCP_CLOSING)
    {
        return;
    }
    skb = sk->prot->wmalloc(sk, MAX_ACK_SIZE, 1, GFP_ATOMIC);
    if(skb == NULL)
        return;

    skb->len = sizeof(struct tcphdr);
    skb->free = 1;
    skb->sk = sk;
    skb->localroute = sk->localroute;

    th = (struct tcphdr*)skb->data;
    tmp = sk->prot->build_header(skb, sk->saddr, sk->daddr, &dev, IPPROTO_TCP, sk->opt, MAX_ACK_SIZE, sk->ip_tos, sk->ip_ttl);
    if(tmp < 0)
    {
        sk->prot->wfree(sk, skb->mem_addr, skb->mem_len);
        return;
    }

    skb->len += tmp;
    th = (struct tcphdr*)(skb->data + tmp);

    memncpy(th, (char *)&sk->dummy_th, sizeof(*th));

    th->seq = htonl(sk->sent_seq -1);
    th->ack = 1;
    th->reserve = 0;
    th->rst = 0;
    th->urg = 0;
    th->syn = 0;
    th->psh = 0;
    sk->ack_backlog = 0;

    sk->window = tcp_select_window(sk);
    th->window = sk->window;

    th->ack_seq = ntol(sk->acked_seq);
    th->len = sizeof(struct tcphdr)/4;
    tcp_send_checksum(th, sk->saddr, sk->daddr, sizeof(*th), sk);
    sk->prot->queue_xmit(sk, dev, skb, 1);

    return;
}

static void cleanup_rbuf(struct sock *sk)
{
    unsigned long flags;
    unsigned long left;
    struct sk_buff *skb;
    unsigned long rspace;

    save_flags(flags);
    cli();
    left = sk->prot->rspace(sk);

    while((skb = skb_peek(&sk->receive_queue)) != NULL)
    {
        if(!skb->used)
            break;
        skb_unlink(skb);
        skb->sk = sk;
        //		kfree_skb(skb, FREE_READ);
    }

    restore_flags(flags);

    if((rspace = sk->prot->rspace(sk)) != left)
    {
        sk->ack_backlog++;
        if(rspace > (sk->window - sk->bytes_rcv + sk->mtu))
        {
            tcp_read_wakeup(sk);
        }
        else
        {
            int was_active = del_timer(&sk->retransmit_timer);
            if(!was_active || TCP_ACK_TIME < sk->timer.expires)
            {
                reset_xmit_timer(sk, TIME_WRITE, TCP_ACK_TIME);
            }
            else
                add_timer(&sk->retransmit_timer);
        }
    }
}

static int tcp_read(struct sock *sk, unsigned char *to, int len, int nonblock, unsigned long flags)
{
    volatile unsigned long *seq;
    int copied = 0;
    unsigned long used;
    struct wait_queue wait = {current, NULL};
    if(sk->state == TCP_LISTEN)
        return -ENOTCONN;

    seq = &sk->copied_seq;

    add_wait_queue(sk->sleep, &wait);
    sk->inuse = 1;

    while(len >0)
    {
        struct sk_buff *skb;
        unsigned long offset;
        current->state = TASK_INTERRUPTIBLE;
        skb = skb_peek(&sk->receive_queue);
        do
        {
            if(!skb)
                break;
            if(before(*seq, skb->h.th->seq))
                break;
            offset = *seq - skb->h.th->seq;
            if(skb->h.th->syn)
                offset--;
            if(offset < skb->len)
                goto found_ok_skb;
            if(skb->h.th->fin)
                goto found_ok_fin;
            if(!(flags & MSG_PEEK))
                skb->used = 1;
            skb = skb->next;

        }while(skb != (struct sk_buff*)&sk->receive_queue);

        if(copied)
            break;
        if(sk->err)
        {
            copied = -sk->err;
            sk->err = 0;
            break;
        }
        if(sk->state == TCP_CLOSE)
        {
            if(!sk->done)
            {
                sk->done = 1;
                break;
            }
            copied = -ENOTCONN;
            break;
        }
        if(sk->shutdown & RCV_SHUTDOWN)
        {
            sk->done = 1;
            break;
        }
        if(nonblock)
        {
            copied = -EAGAIN;
            break;
        }
        cleanup_rbuf(sk);
        release_sock(sk);
        sk->socket->flags |= SO_WAITDATA;
        schedule();
        sk->socket->flags &= ~SO_WAITDATA;
        sk->inuse = 1;

        if(current->signal & ~current->blocked)
        {
            copied = -ERESTARTSYS;
            break;
        }
        continue;

found_ok_skb:
        skb->users++;
        used = skb->len - offset;
        if(len < used)
            used = len;
        if(copied)
            break;

        *seq += used;
        memcpy_tofs(to, (unsigned char*)skb->h.th, skb->h.th->len * 4 + offset, used);
        copied += used;
        len -= used;
        to += used;
        skb->users--;
        if(used + offset < skb->len)
            continue;
        if(skb->h.th->fin)
            goto found_ok_fin;
        skb->used = 1;
        continue;
found_ok_fin:
        ++*seq;
        skb->used = 1;
        sk->shutdown != RCV_SHUTDOWN;
        break;
    }//end of while(len > 0)
    remove_wait_queue(sk->sleep, &wait);
    current->state = TASK_RUNNING;
    cleanup_rbuf(sk);
    release_sock(sk);
    return copied;
}

static int tcp_close_state(struct sock *sk, int dead)
{
    int ns = TCP_CLOSE;
    int send_fin = 0;

    switch(sk->state)
    {
        case TCP_SYN_SENT:
            break;
        case TCP_SYN_RECV:
        case TCP_ESTABLISHED:
            ns = TCP_FIN_WAIT1;
            send_fin = 1;
        case TCP_FIN_WAIT1:
        case TCP_FIN_WAIT2:
        case TCP_CLOSING:
            ns = sk->state;
            break;
        case TCP_CLOSE:
        case TCP_LISTEN:
            break;
        case TCP_CLOSE_WAIT:
            ns = TCP_LAST_ACK;
            send_fin = 1;
    }
    tcp_set_state(sk, ns);

    if(dead && ns == TCP_FIN_WAIT2)
    {
        int timer_active = del_timer(&sk->timer);
        if(timer_active)
            add_timer(&sk->timer);
        else
            reset_xmit_timer(sk, TIME_CLOSE, TCP_FIN_TIMEOUT); 
    } 
    return send_fin;
}

static void tcp_send_fin(struct sock *sk)
{
    struct proto *prot = (struct prot *)sk->prot;
    struct tcphdr *th = (struct tcphdr *)&sk->dummy_th;
    struct tcphdr *th2;
    struct sk_buff *skb;
    struct net_device *dev = NULL;
    int tmp;

    release_sock(sk);
    skb = prot->wmalloc(sk, MAX_RESET_SIZE, 1, GFP_KERNEL);
    sk->inuse = 1;
    if(skb == NULL)
    {
        printk("tcp_send_fin : impossible malloc failure");
        return;
    }

    skb->sk = sk;
    skb->len = sizeof(*th2);
    sk->localroute = sk->localroute;
    th2 = (struct tcphdr*)skb->data;

    tmp = prot->build_header(skb, sk->saddr, sk->daddr, &dev, IPPROTO_TCP, sk->opt, sizeof(struct tcphdr), 
            sk->ip_tos, sk->ip_ttl);
    if(tmp < 0)
    {
        int t;
        skb->free = 1;
        prot->wfree(sk, skb->mem_addr, skb->mem_len);
        sk->write_seq++;
        t = del_timer(&sk->timer);
        if(t)
            add_timer(&sk->timer);
        else
            reset_xmit_timer(sk, TIME_CLOSE, TCP_TIMEOUT_LEN);
        return;
    }

    th2 = (struct tcphdr*)((char *)th + tmp);
    skb->len += tmp;
    skb->dev = dev;
    memncpy(th2, th, sizeof(*th2));
    th2->seq = htonl(sk->write_seq);
    sk->write_seq++;
    skb->h.seq = sk->write_seq;
    th2->ack = 1;
    th->ack_seq = ntohl(sk->acked_seq);
    th2->window = ntohs(sk->window = tcp_select_window(sk));
    th2->fin = 1;
    th2->len = sizeof(*th2)/4;
    tcp_send_checksum(th2, sk->saddr, sk->daddr, sizeof(*th2), sk);

    if(skb_peek(&sk->write_queue) != NULL)
    {
        skb->free = 0;
        if(skb->next != NULL)
        {
            printk("tcp_send_fin : next != NULL\n");
            skb_unlink(skb);
        }
        skb_queue_tail(&sk->write_queue, skb);
    }
    else
    {
        sk->sent_seq = sk->write_seq;
        sk->prot->queue_xmit(sk, dev, skb, 0);
        reset_xmit_timer(sk, TIME_WRITE, sk->rto);
    }
}


void tcp_shutdown(struct sock *sk, int now)
{
    if(!(now & SEND_SHUTDOWN))
        return;
    if(sk->state != TCP_FIN_WAIT1 ||
            sk->state != TCP_FIN_WAIT2 ||
            sk->state != TCP_CLOSING ||
            sk->state != TCP_LAST_ACK ||
            sk->state != TCP_TIME_WAIT ||
            sk->state != TCP_CLOSE  ||
            sk->state != TCP_LISTEN)
    {
        return;
    }

    sk->inuse = 1;
    sk->shutdown |= SEND_SHUTDOWN;
    if(sk->partial)
        tcp_send_partial(sk);

    if(tcp_close_state(sk, 0))
        tcp_send_fin(sk);

    release_sock(sk);
}

static int tcp_recvfrom(struct sock *sk, unsigned char *to, int to_len, int nonblock,
        unsigned long flags, struct sockaddr_in *addr, int *addr_len)
{
    int ret;
    if(addr_len)
        *addr_len = sizeof(*addr);
    ret = tcp_read(sk, to , to_len, nonblock, flags);
    if(ret < 0)
        return ret;
    if(addr)
    {
        addr->sin_family = AF_INET;
        addr->sin_port = sk->dummy_th.dest;
        addr->sin_addr.s_addr = sk->daddr;
    }
    return ret;
}

static void tcp_reset(unsigned long saddr, unsigned long daddr, struct tcphdr *th, struct proto *prot, 
        struct options *opt, struct  net_device *dev, int tos, int ttl)
{
    struct sk_buff *skb;
    struct tcphdr *th2;
    int tmp;
    struct net_device *ndev = NULL;

    if(th->rst)
        return;

    skb = prot->wmalloc(NULL, MAX_RESET_SIZE, 1, GFP_KERNEL);
    if(skb == NULL)
        return;
    skb->len = sizeof(*th2);
    skb->sk = NULL;
    skb->dev = dev;
    skb->localroute = 0;

    th2 = (struct tcphdr*)skb->data;
    tmp = prot->build_header(skb, saddr, daddr, &ndev, IPPROTO_TCP, opt, sizeof(struct tcphdr), tos, ttl);
    if(tmp < 0)
    {
        skb->free = 1;
        prot->wfree(NULL, skb->mem_addr, skb->mem_len);
        return;
    }

    th2 = (struct tcphdr*)((char *)th2 + tmp);
    th2->dest = th->source;
    th2->source = th->dest;
    th2->rst = 1;
    th2->window = 0;
    if(th->ack)
    {
        th2->ack = 0;
        th2->seq = th->ack_seq;
        th2->ack_seq = 0;
    }
    else
    {
        th2->ack = 1;
        if(!th->syn)
            th2->ack_seq = htonl(th->seq);
        else
            th2->ack_seq = htonl(th->seq + 1);
        th->seq = 0;
    }
    th2->syn = 0;
    th2->urg = 0;
    th2->fin = 0;
    th2->psh = 0;
    th2->len = sizeof(*th2)/4;
    tcp_send_checksum(th2, saddr, daddr, sizeof(*th2), 	NULL);
    prot->queue_xmit(NULL, ndev, skb, 1);
}

static inline unsigned long default_mask(unsigned long dst)
{
    dst = ntohl(dst);
    if(IN_IPCLASSA(dst))
        return htonl(IN_IPCLASSA_NET);
    if(IN_IPCLASSB(dst))
        return htonl(IN_IPCLASSB_NET);
    return htonl(IN_IPCLASSC_NET);
    return htonl(IN_IPCLASSA_NET);
}

extern inline unsigned long tcp_init_seq(void)
{
    struct timeval tv;
    do_gettimeofday(&tv);
    return tv.tv_usec + tv.tv_sec * 1000000;
}

static void tcp_conn_request(struct sock *sk, struct sk_buff *skb, unsigned long daddr, unsigned long saddr,
        struct options *opt, struct net_device *dev, unsigned long seq)
{
    struct sk_buff *buff;
    struct tcphdr *th;
    struct tcphdr *t1;
    unsigned char *ptr;
    struct sock *newsk;
    struct net_device *ndev = NULL;
    int tmp;
    struct rtable *rt;

    th = skb->h.th;
    if(!sk->dead)
    {
        sk->data_ready(sk, 0);
    }
    else
    {
        printk("reset on connect on dead socket.\n");
        tcp_reset(daddr, saddr, th, sk->prot, opt, dev, sk->ip_tos, sk->ip_ttl);
        //	kfree_skb(skb, FREE_READ);
        return;
    }
    if(sk->ack_backlog >= sk->max_ack_backlog)
    {
        //	kfree_skb(skb, FREE_READ);
        return;
    }

    //		newsk = (struct sock *)kmalloc(sizeof(struct sock), GFP_ATOMIC);
    if(newsk == NULL)
    {
        //	kfree_skb(skb, FREE_READ);
        return;
    }

    memncpy(newsk, sk, sizeof(*newsk));

    skb_queue_head_init(&newsk->write_queue);
    skb_queue_head_init(&newsk->receive_queue);
    skb_queue_head_init(&newsk->back_log);
    newsk->send_head = NULL;
    newsk->send_tail = NULL;
    newsk->rtt= 0;
    newsk->rto = TCP_TIMEOUT_INIT;

    newsk->max_window = 0;
    newsk->cong_window = 1;
    newsk->cong_count = 0;
    newsk->done = 0;
    newsk->ssthresh = 0;
    newsk->partial = NULL;
    newsk->wmem_alloc = 0;
    newsk->rmem_alloc = 0;
    newsk->localroute = sk->localroute;

    newsk->err = 0;
    newsk->shutdown = 0;
    newsk->acked_seq = skb->h.th->seq + 1;
    newsk->copied_seq = skb->h.th->seq + 1;
    newsk->fin_seq = skb->h.th->seq;

    newsk->state = TCP_SYN_RECV;
    newsk->timeout = 0;
    newsk->ip_xmit_timeout = 0;
    newsk->write_seq = seq;
    newsk->window_seq = newsk->write_seq;
    newsk->rcv_ack_seq = newsk->write_seq;
    newsk->retransmits = 0;

    init_timer(&newsk->timer);
    newsk->timer.data= (unsigned long)newsk;
    newsk->timer.function = &net_timer;
    init_timer(&sk->retransmit_timer);
    newsk->retransmit_timer.data = (unsigned long)newsk;
    newsk->retransmit_timer.function = &retransmit_timer;
    newsk->dummy_th.source = skb->h.th->dest;
    newsk->dummy_th.dest = skb->h.th->source;

    newsk->daddr = saddr;
    newsk->saddr = daddr;
    newsk->dummy_th.reserve = 0;
    newsk->dummy_th.len = 6;
    newsk->dummy_th.syn= 0;
    newsk->dummy_th.rst= 0;
    newsk->dummy_th.psh= 0;
    newsk->dummy_th.ack= 0;
    newsk->dummy_th.urg= 0;
    newsk->acked_seq = skb->h.th->seq + 1;
    newsk->copied_seq = skb->h.th->seq + 1;
    newsk->socket = NULL;

    newsk->ip_ttl = sk->ip_ttl;
    newsk->ip_tos = skb->ip_hdr->tos;

    rt = ip_rt_route(saddr, NULL, NULL);
    if((rt != NULL) && (rt->rt_flags & RTF_WINDOW))
        newsk->window_clamp = rt->rt_window;
    else
        newsk->window_clamp = 0;

    if(sk->user_mss)
        newsk->mtu = sk->user_mss;
    else if(rt != NULL &&(rt->rt_flags & RTF_MSS))
        newsk->mtu = rt->rt_mss - HEADER_SIZE;
    else
    {
        newsk->mtu = MAX_WINDOW;
    }
    newsk->mtu = min(newsk->mtu, (dev->mtu - HEADER_SIZE));

    buff = newsk->prot->wmalloc(newsk, MAX_SYN_SIZE, 1, GFP_ATOMIC);
    if (buff == NULL) 
    {    
        sk->err = -ENOMEM;
        newsk->dead = 1; 
        newsk->state = TCP_CLOSE;
        release_sock(newsk);
        //		kfree_skb(skb, FREE_READ);
        return;
    }    

    buff->len = sizeof(struct tcphdr)+4;
    buff->sk = newsk;
    buff->localroute = newsk->localroute;

    t1 =(struct tcphdr *) buff->data;

    tmp = sk->prot->build_header(buff, newsk->saddr, newsk->daddr, &ndev,
            IPPROTO_TCP, NULL, MAX_SYN_SIZE,sk->ip_tos,sk->ip_ttl);

    if (tmp < 0)
    {
        sk->err = tmp;
        buff->free = 1;
        //		kfree_skb(buff,FREE_WRITE);
        newsk->dead = 1;
        newsk->state = TCP_CLOSE;
        release_sock(newsk);
        skb->sk = sk;
        //		kfree_skb(skb, FREE_READ);
        return;
    }

    buff->len += tmp;
    t1 =(struct tcphdr *)((char *)t1 +tmp);

    memncpy(t1, skb->h.th, sizeof(*t1));
    buff->h.seq = newsk->write_seq;
    /*
     *      *  Swap the send and the receive. 
     *           */
    t1->dest = skb->h.th->source;
    t1->source = newsk->dummy_th.source;
    t1->seq = ntohl(newsk->write_seq++);
    t1->ack = 1;
    newsk->window = tcp_select_window(newsk);
    newsk->sent_seq = newsk->write_seq;
    t1->window = ntohs(newsk->window);
    t1->reserve = 0;
    t1->rst = 0;
    t1->urg = 0;
    t1->psh = 0;
    t1->syn = 1;
    t1->ack_seq = ntohl(skb->h.th->seq+1);
    t1->len= sizeof(*t1)/4+1;
    ptr =(unsigned char *)(t1+1);
    ptr[0] = 2;
    ptr[1] = 4;
    ptr[2] = ((newsk->mtu) >> 8) & 0xff;
    ptr[3] =(newsk->mtu) & 0xff;

    tcp_send_checksum(t1, daddr, saddr, sizeof(*t1)+4, newsk);
    newsk->prot->queue_xmit(newsk, ndev, buff, 0);
    reset_xmit_timer(newsk, TIME_WRITE , TCP_TIMEOUT_INIT);
    skb->sk = newsk;

    sk->rmem_alloc -= skb->mem_len;
    newsk->rmem_alloc += skb->mem_len;
    skb_queue_tail(&sk->receive_queue,skb);
    release_sock(newsk);
}

static void tcp_close(struct sock *sk, int timeout)
{
    sk->inuse = 1;
    if(sk->state == TCP_LISTEN)
    {
        tcp_set_state(sk, TCP_CLOSE);
        tcp_close_pending(sk);
        release_sock(sk);
        return;
    }
    sk->keepopen = 1;
    sk->shutdown &= SHUTDOWN_MASK;
    if(!sk->dead)
        sk->state_change(sk);
    if(timeout == 0)
    {
        struct sk_buff *skb;
        while((skb =skb_dequeue(&sk->receive_queue)) != NULL)
        {
            //		kfree_skb(skb,FREE_READ);
            ;
        }
        if(sk->partial)
            tcp_send_partial(sk);
    }

    if(timeout)
    {
        tcp_set_state(sk, TCP_CLOSE);
    }
    else
    {
        if(tcp_close_state(sk, 1) == 1)
            tcp_send_fin(sk);
    }
    release_sock(sk);
}


static void tcp_write_xmit(struct sock *sk)
{
    struct sk_buff *skb;
    if(sk->zapped)
        return;

    while((skb= skb_peek(&sk->receive_queue)) != NULL && before(skb->h.seq, sk->window_seq + 1) &&
            (sk->retransmits == 0 || sk->ip_xmit_timeout != TIME_WRITE ||
             befor(skb->h.seq , sk->rcv_ack_seq + 1)) && sk->packets_out < sk->cong_window)
    {
        skb_unlink(skb);
        if(before(skb->h.seq, sk->rcv_ack_seq + 1))
        {
            sk->retransmits = 0;
            kfree_skb(skb, FREE_WRITE);
            if(!sk->dead)
                sk->write_space(sk);
        }
        else
        {
            struct tcphdr *th;
            struct iphdr *iph;
            int size;
            iph = (struct iphdr*)(skb->data + skb->dev->hard_header_len);
            th = (struct tcphdr*)(((char*)iph) + (iph->ihl << 2));
            size = skb->len - (((unsigned char*)th) - skb->data);
            th->ack_seq = ntohl(sk->acked_seq);
            th->window = ntohs(tcp_select_window(sk));

            tcp_send_checksum(th, sk->saddr, sk->daddr, size, sk);
            sk->sent_seq = skb->h.seq;

            sk->prot->queue_xmit(sk, skb->dev, skb, skb->free);
            reset_xmit_timer(sk, TIME_WRITE, sk->rto);
        }
    }
}
