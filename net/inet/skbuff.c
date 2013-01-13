#include "type.h"
#include "const.h"
#include "stddef.h"
#include "asm-i386/system.h"
#include "linux/skbuff.h"
#include "printf.h"


void skb_queue_head_init(struct sk_buff_head *list)
{
		list->prev = (struct sk_buff *)list;
		list->next = (struct sk_buff *)list;
}

struct sk_buff* skb_dequeue(struct sk_buff_head *list_)
{
		struct sk_buff *rst = list_->next;

		struct sk_buff *list = (struct sk_buff*)list_;

		if(rst == list)
				return NULL;

		list->next = rst->next;
		rst->next->prev = list;

		rst->prev = NULL;
		rst->next = NULL;

		return rst;
}

void dev_kfree_free(struct sk_buff *skb,int mode)
{
		return;
}


struct sk_buff* alloc_skb(unsigned int size,int priority)
{
		struct sk_buff *skb = NULL;
		return skb;
}

void skb_queue_head(struct sk_buff_head *skh,struct sk_buff *skb)
{
		struct sk_buff *list = (struct sk_buff *)skh;

		skb->next = list->next;
		skb->prev = list;
		
		skb->next->prev = skb;
		skb->prev->next = skb;

		return;
}

void skb_queue_tail(struct sk_buff_head *skh,struct sk_buff *skb)
{
	struct sk_buff *list = (struct sk_buff *)skh;

	skb->next = list;
	skb->prev = list->prev;

	skb->prev->next = skb;
	skb->next->prev = skb;
}

void skb_unlink(struct sk_buff *skb)
{
		unsigned long flags;
		save_flags(flags);
		if(skb->prev && skb->next)
		{
				skb->prev->next = skb->next;
				skb->next->prev = skb->prev;
				skb->next = NULL;
				skb->prev = NULL;
		}
		else
				printk("skb_unlink : not a linked elment\n");
		restore_flags(flags);
		return;
}
