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
#include "bitops.h"
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

unsigned long free_list_init (unsigned long start_mem, unsigned long end_mem)
{
    int i;
    for(i = 0;i < NR_MEM_LISTS; i++)
    {
        unsigned long bitmap_size;
        bitmap_size = BITMAP_MAX >> i;
        free_mem_list[i].prev = free_mem_list[i].next = &free_mem_list[i];
        free_mem_map[i] = (unsigned char *)start_mem;
        memset((void *)start_mem, 0, bitmap_size);
        start_mem += bitmap_size;
    }
    return start_mem;
}

unsigned long paging_init(unsigned long start_mem, unsigned long end_mem)
{
    int tmp;

    pgd_t *pg_dir;
    pte_t *pg_table;

    unsigned long address;

    pg_dir = swapper_pg_dir;
    address = 0;

    while(address < end_mem)
    {
        pg_table = (pte_t *)(pgd_val(pg_dir[768]));
        start_mem += PAGE_SIZE;
        for(tmp = 0;tmp < PTRS_PER_PTE; tmp++,pg_table++)
        {
            if(address < end_mem)
                *pg_table =  mk_pte(address,PAGE_SHARED);
            address += PAGE_SIZE;
        }
        pg_dir++;

    }
    free_list_init(start_mem, end_mem);
    return 0;
}

unsigned long get_free_pages(unsigned long order)
{
    struct mem_list *queue = free_mem_list + order;
    unsigned long new_order = order;
    do
    {
        struct mem_list *next = queue->next;
        if(queue != next)
        {
            queue->next = next->next;
            queue->next->prev = queue;
            nr_free_pages -= (1 << order);
            return (unsigned long)next;
        }
        new_order++;
        queue++;

    }while(new_order < NR_MEM_LISTS);
    return 0;
}

unsigned long __get_free_pages(int priority, unsigned long order)
{
    if(priority == GFP_ATOMIC)
        return get_free_pages(order);
    return 0;
}

static inline void add_mem_queue (struct mem_list *head, struct mem_list *entry)
{
    entry->prev = head;
    head->next->prev = entry;
    entry->next = head->next;
    head->next = entry;

}

static inline void remove_mem_queue (struct mem_list *head, struct mem_list *entry)
{
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
}

static inline void free_pages_ok(unsigned long addr, unsigned long order)
{
    add_mem_queue(free_mem_list + order, (struct mem_list *)addr);
}

void free_pages(unsigned long addr, unsigned long order)
{
    if(addr < main_memory_end)
    {
        struct page *map = mem_map + MAP_NR(addr);
        if(map)
        {
            if(!(map->flags & MAP_PAGE_RESERVED))
            {
                free_pages_ok(addr, order);
            }
            return;

        }
        printk("Trying to free free memory (%081x):memory probably corrupted\n",addr);
        return;
    }
}

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
    while(set_bit(offset, p->lock_map))
        interruptible_sleep_on(&lock_queue);
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

    if (offset && !clear_bit(offset,p->lock_map))
        printk("rw_swap_page: lock already cleared\n");
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
