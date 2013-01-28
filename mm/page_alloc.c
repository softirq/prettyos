#include "type.h"
#include "const.h"
#include "string.h"
#include "stdlib.h"
#include "panic.h"
#include "printf.h"
#include "list.h"
#include "math.h"
#include "mm.h"
#include "page_alloc.h"

static inline int free_pages_ok(struct page *page, const int order)
{
    if(page == NULL || order >= NR_MEM_LISTS)
        return -1;

    /* insert into buddy list by address sort */
    if(buddy_list_add_line(page, order) < 0)
        return -2;

    return 0;
}

int free_pages(struct page *page, const int order)
{
    if(page == NULL || order >= NR_MEM_LISTS)
        return -1;

    if(page->flags & SYS_RAM)
    {
        if(free_pages_ok(page, order) < 0)
            return -2;
    }

    return 0;
}

unsigned long get_free_pages(int order)
{
    if(order >= NR_MEM_LISTS)
        return NULL;

    struct buddy_list *queue = NULL;
    struct page *page = NULL;
    struct list_head *head = NULL, *item = NULL;
    int new_order = 0;

repeat:

    queue = buddy_list + order;
    new_order = order;
    if(queue->nr_free_pages > 0)
    {
        head = &(queue->list);
        if(buddy_list_del(head, order, &item) != 0)
            return -3;
        page = list_entry(item,Page,list);
        return page_address(page);
        /*buddy_list_add(page, order);*/
    }
    else
    {
        while(queue->nr_free_pages == 0)
        {
            ++new_order;
            ++queue;
        }
        /*printk("new_order=%x.",new_order);*/

        head = &(queue->list);
        if(buddy_list_del(head, new_order, &item) != 0)
            return -2;
        page = list_entry(item, Page, list);
        --new_order;
        buddy_list_add(page, new_order);
        page += power(new_order);
        buddy_list_add(page, new_order);
        goto repeat;
    }

    return NULL;
}

unsigned long __get_free_pages(const int priority, const int order)
{
    if(priority == GFP_ATOMIC)
        return get_free_pages(order);

    return 0;
}

unsigned long get_free_page(const int priority)
{
    unsigned long address; 
    address = __get_free_page(priority);
    memset((void *)address, 0 ,PAGE_SIZE);

    return address;
}

unsigned long alloc_mem(const size_t size)
{
    int order = power(size>>PAGE_SHIFT);
    return get_free_pages(order);
}
