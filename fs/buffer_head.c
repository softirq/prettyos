#include "type.h"
#include "const.h"
#include "irq.h"
#include "wait.h"
#include "mm.h"
#include "page_alloc.h"
#include "panic.h"
#include "stdlib.h"
#include "hd.h"
#include "blk_drv.h"
#include "fcntl.h"
#include "printf.h"
#include "buffer_head.h"
#include "math.h"

/*a hash and a free list*/

//long end =  3 * 1024 * 1024;
struct buffer_head *start_buffer;	// = (struct buffer_head *)(&end);
struct buffer_head *hash_buffer[NR_HASH_BUFFER];
static struct buffer_head *buffer_free_list;
static struct wait_queue *buffer_wait = NULL;
int NR_BUFFERS = 0;

#define _hashfn(dev,block)	(((unsigned)(dev^block))%NR_HASH_BUFFER)
#define  hash(dev,block)	hash_buffer[_hashfn(dev,block)]

//remove from hash-queue and free-list
static inline void remove_from_queues(struct buffer_head *bh)
{

    if(bh->b_next)
        bh->b_next->b_prev = bh->b_prev;
    if(bh->b_prev)
        bh->b_prev->b_next = bh->b_next;

    if(hash(bh->b_dev,bh->b_blocknr) == bh)
        hash(bh->b_dev,bh->b_blocknr) = bh->b_next;

    //free list是个双向链表
    if(!(bh->b_prev_free) || !(bh->b_next_free))	
        panic("free block list corrupted");	
    bh->b_prev_free->b_next_free = bh->b_next_free;
    bh->b_next_free->b_prev_free = bh->b_prev_free;
    if(buffer_free_list == bh)
        buffer_free_list = bh->b_next_free;
}

static inline void insert_into_queues(struct buffer_head *bh)
{
    //put at end of free-list
    bh->b_next_free = buffer_free_list;
    bh->b_prev_free = buffer_free_list->b_next_free;
    buffer_free_list->b_prev_free->b_next_free = bh;

    //put the buffer in new hash-queue if  it has a device
    bh->b_prev = NULL;
    bh->b_next = NULL;
    if(!bh->b_dev)
        return;
    bh->b_next = hash(bh->b_dev,bh->b_blocknr);
    hash(bh->b_dev,bh->b_blocknr) = bh;
    bh->b_next->b_prev = bh;
}

static void wait_on_buffer(struct buffer_head *bh)
{
    while(bh->b_lock)
    {
        sleep_on(&bh->b_wait);
    }
}

static inline void lock_buffer(struct buffer_head *bh)
{
    disable_int();
    while(bh->b_lock)
        sleep_on(&bh->b_wait);
    bh->b_lock = 1;
    enable_int();
}

static inline void unlock_buffer(struct buffer_head *bh)
{
    if(!bh->b_lock)
        panic("buffer.c:buffer not locked\n");
    bh->b_lock = 0;
    wake_up(&bh->b_wait);
}

static struct buffer_head * find_buffer(int dev,int block)
{
    struct buffer_head *bh = NULL;

    for(bh = hash(dev,block);bh != NULL;bh = bh->b_next)
    {
        if(bh->b_dev == dev && bh->b_blocknr == block)
            return bh;
    }

    return NULL;
}

static struct buffer_head * get_hash_buffer(int dev,int block)
{
    struct buffer_head *bh;
    for(;;)
    {
        if(!(bh = find_buffer(dev,block)))	
            return NULL;	
        bh->b_count++;
        wait_on_buffer(bh);
        if(bh->b_dev == dev && bh->b_blocknr == block)
            return bh;
        bh->b_count--;
    }

    return NULL;
}

struct buffer_head *bread(int dev,int block_nr)
{
    struct buffer_head *bh;
    if(!(bh = getblk(dev,block_nr)))
        panic("bread:getblk returned NULL\n");
    if(bh->b_uptodate)
        return bh;
    hd_rw(dev,block_nr,1,ATA_READ,bh);
    wait_on_buffer(bh);
    if(bh->b_uptodate)
        return bh;
    brelse(bh);
    return NULL;
}

//buffer_head的权值(选择最佳的buffer_head)
#define BADNESS(bh) (((bh)->b_dirt << 1) + (bh)->b_blocknr)
/* get dev the nr_block block*/
struct buffer_head * getblk(int dev,int block)
{
    /*printk("getblk -------------------------------- \n");*/
    struct buffer_head *item,*bh;
repeat:
    if((bh = get_hash_buffer(dev,block)))
    {
        return bh;
    }
    else
    {
        /*printk("not found.");*/
    }

    item= buffer_free_list;
    do{
        if(item->b_count)
            continue;
        if(!bh || (BADNESS(bh) > BADNESS(item)))
        {
            bh = item;
            if(!BADNESS(item))
            {
                break;
            }
        }
    }while((item = item->b_next_free) != buffer_free_list);

    if(!bh)
    {
        sleep_on(&buffer_wait);
        goto repeat;		
    }

    /*printk("b_lock = %d", bh->b_lock);*/
    wait_on_buffer(bh);
    if(bh->b_count)
    {
        goto repeat;
        /*return NULL;*/
    }

#if 0
    /*	while(bh->b_dirt)*/
    /*{*/
    /*sync_dev(bh->b_dev);*/
    /*wait_on_buffer(bh);*/
    /*if(bh_b_count)*/
    /*goto repeat;*/
    /*}		*/
    /**/
#endif

    /*///已经被其他的进程取走*/
    if(find_buffer(dev,block))
    {
        goto repeat;	
    }

    bh->b_count = 1;
    bh->b_dirt = 0;
    bh->b_uptodate = 0;
    remove_from_queues(bh);
    bh->b_dev = dev;
    bh->b_blocknr = block;

    insert_into_queues(bh);

    return bh;

}
#undef BADNESS

void brelse(struct buffer_head *bh)
{
    if(!bh)
        return;
    wait_on_buffer(bh);
    if(!(bh->b_count--))
        panic("trying to free free buffer");
    //	wake_up(&buffer_wait);
}

void buffer_init()
{
    struct buffer_head *bh = NULL;
    void *b = NULL;
    int i = 0;

    unsigned long start_buffer = get_free_pages(BUFFER_HEAD_PAGE); 
    unsigned long end_buffer = start_buffer + power(BUFFER_HEAD_PAGE) * PAGE_SIZE;
    bh = (struct buffer_head *)start_buffer;

    b = (void *)end_buffer;

    while((b -= SECTOR_SIZE) >= (void *)(bh + 1))
    {
        bh->b_dev = 0;
        bh->b_dirt = 0;
        bh->b_count = 0;
        bh->b_lock = 0;
        bh->b_uptodate = 0;
        bh->b_wait = NULL;
        bh->b_prev = NULL;
        bh->b_next = NULL;
        bh->b_data = (char *)b;
        bh->b_prev_free = bh - 1;
        bh->b_next_free = bh + 1;	
        bh++;
        NR_BUFFERS++;
    }
    //让h指向最后一个有效的缓冲头
    bh--;
    //形成双向链表
    buffer_free_list = (struct buffer_head *)start_buffer;
    buffer_free_list->b_prev_free = bh;
    bh->b_next_free = buffer_free_list;

    for(i = 0;i < NR_HASH_BUFFER;i++)
    {
        hash_buffer[i] = NULL;
    }
}
