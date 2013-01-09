#include "type.h"
#include "const.h"
#include "traps.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "panic.h"
#include "printf.h"
#include "fork.h"
#include "stdlib.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "global.h"
#include "pgtable.h"
#include "swap.h"
#include "bitmap.h"
#include "kstat.h"
#include "proc.h"

/*
 * buddy system
 */

#define BITMAP_MAX 		(1024)

static int nr_swapfiles = 0;
static struct wait_queue *lock_queue = NULL;
static int swap_cache_add_total = 0;

struct swap_info_struct swap_files[MAX_SWAPFILES];
/*struct page pages[];*/


/*static inline void remove_mem_queue (struct mem_list *head, struct mem_list *entry)
{
    [>entry->prev->next = entry->next;<]
    [>entry->next->prev = entry->prev;<]
}*/

void add_to_swap_cache (struct page *page, swap_entry_t entry)
{
    swap_cache_add_total++;
}

void swap_free(swap_entry_t entry)
{
    struct swap_info_struct *p;
    unsigned long offset, type;

    if(!entry.val)
        return;
    type = SWAP_TYPE(entry);
    //		if(type == SHM_SWP_TYPE)
    //				return;
    if(type >= nr_swapfiles)
    {
        printk("Trying to free nonexistent swap-page\n");
        goto bad_nofile;
    }
    p = &swap_files[type];
    offset = SWAP_OFFSET(entry);
    if(offset >= p->max)
    {
        printk("swap_free: weirdness\n");
        goto bad_offset;
    }
    if(p->swap_map[offset] < SWAP_MAP_MAX)
    {
        if(offset < p->lowest_bit)
            p->lowest_bit = offset;
        if(offset < p->highest_bit)
            p->highest_bit = offset;
        nr_swap_pages++;
    }

bad_nofile:
bad_offset:
    return;
}

void rw_swap_page(int rw, swap_entry_t entry, char *buf)
{
    unsigned long offset, type;
    struct swap_info_struct *p;

    type = SWAP_TYPE(entry);
    if(type >= nr_swap_pages)
    {
        printk("Trying to free nonexistent swap-page\n");
        goto bad_nofile;
    }
    offset = SWAP_OFFSET(entry);

    if(offset >= p->max)
    {
        printk("swap_free: weirdness\n");
        goto bad_offset;
    }
    if(p->swap_map && !p->swap_map[offset])
    {
        printk("Trying to use unallocated swap (%08lx)\n", entry.val);
        return;
    }
    /*while(__set_bit(offset, p->lock_map))*/
        /*interruptible_sleep_on(&lock_queue);*/
    if(rw == READ)
        kstat.pswpin++;
    else 
        kstat.pswpout++;
    if(p->swap_device)
    {
        //		ll_rw_page(rw, p->swap_device, offset, buf);
    }
    else if(p->swap_file)
    {
        /*struct inode *swapf = p->swap_file;*/
        //		ll_rw_swap_file(rw,swapf->i_dev, zones, i,buf);
    }
    else
        printk("re_swap_page: no swap file or device\n");

    /*if (offset && !__clear_bit(offset,p->lock_map))*/
        /*printk("rw_swap_page: lock already cleared\n");*/
    wake_up_interruptible(&lock_queue);

bad_nofile:
bad_offset:
    return;
}

void swap_in(struct vm_area_struct * vma, pte_t * page_table,
        swap_entry_t entry, int write_access)
{
    unsigned long page;

    if(pte_val(*page_table) != entry.val)
    {
        return;
    }
    page = get_free_page(GFP_KERNEL);
    if(!page)
    {
        pte_val(*page_table) = PAGE_BAD;
        swap_free(entry);
        oom();
    }

    read_swap_page(entry,(char *)page);
    if(pte_val(*page_table) != entry.val)
    {
        return;
    }
    return ;
}
