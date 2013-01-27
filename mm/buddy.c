/* buddy system  */
#include "type.h"
#include "const.h"
#include "string.h"
#include "stdlib.h"
#include "panic.h"
#include "printf.h"
#include "list.h"
#include "math.h"
#include "mm.h"

/*static int count = 0;*/
int nr_free_pages = 0;
unsigned long page_fns = 0;

struct buddy_list buddy_list[NR_MEM_LISTS];

/* print buddy list info */
int print_buddy_list()
{
    int i = 0;
    struct buddy_list *queue = NULL;

    for(i = 0;i < NR_MEM_LISTS; i++)
    {
        queue = buddy_list + i;
        printk("(%d  %d)  ", i, queue->nr_free_pages);
    }

    return 0;
}

/*is page address is closed*/
int is_pageclosed(struct list_head *prev, struct list_head *next, int order)
{
    int index = 1;
    struct page *prev_page = NULL,*next_page = NULL;

    if(prev == NULL || next == NULL)
        return 0;

    prev_page = list_entry(prev, Page, list);
    next_page = list_entry(next, Page, list);

    index = power(order);

    return ((prev_page->address + index * PAGE_SIZE == next_page->address) || (prev_page->address - index * PAGE_SIZE == next_page->address));
}

int buddy_list_del(struct list_head *head, const int order, struct list_head **item)
{
    if(head == NULL || order >= NR_MEM_LISTS || item == NULL)
    {
        return -1;
    }

    if(list_get_del(head, item) != 0)
    {
        return -2;
    }

    --buddy_list[order].nr_free_pages;
    return 0;
}

int buddy_list_add(struct page *page, const int order)
{
    if(page == NULL || order >= NR_MEM_LISTS)
    {
        return -1;
    }

    struct buddy_list *header = buddy_list + order;
    list_add(&(page->list),&(header->list));
    ++buddy_list[order].nr_free_pages;

    return 0;
}

int buddy_list_add_line(struct page *page, const int order)
{
    if(page == NULL || order >= NR_MEM_LISTS)
    {
        printk("=");
        return -1;
    }

    struct buddy_list *header = buddy_list + order;
    struct list_head *head, *pos, *n;
    struct page *item_page = NULL;

    head = &(header->list);
    if(list_empty_careful(head))
    {
        list_add(&(page->list),&(header->list));
        ++buddy_list[order].nr_free_pages;
        return 0;
    }
    else
    {
        list_for_each_safe(pos, n, head)
        {
            item_page = list_entry(pos, Page, list);
            if(page->address < item_page->address)
            {
                __list_add(&(page->list), pos->prev, pos);
                ++buddy_list[order].nr_free_pages;
                return 0;
            }

            if(pos->next == head)
            {
                __list_add(&(page->list), pos, n);
                ++buddy_list[order].nr_free_pages;
                return 0;
            }
        }
    }

    return -1;
}

void buddy_list_init ()
{
    int i;
    for(i = 0;i < NR_MEM_LISTS; i++)
    {
        INIT_LIST_HEAD(&(buddy_list[i].list));
        buddy_list[i].nr_free_pages = 0;
    }
}

/*tidy the buddy list*/
int buddy_list_tidy()
{
    int i ;
    struct buddy_list *queue = NULL;
    struct page *page = NULL;
    struct list_head *head, *pos ,*n;

    for(i = 0;i < NR_MEM_LISTS - 1; i++)
        /*for(i = 0;i < 3; i++)*/
    {
        queue = buddy_list + i;
        /*printk("1 i = %d nr_free_pages = %d.\n", i, queue->nr_free_pages);*/

        if(queue->nr_free_pages <= 0)
            return 0;

        /*list_for_each_safe(pos, n, head)*/

        head = &(queue->list);
        pos = head->next;

        while(pos && pos != head)
        {
            n = pos->next;
            if(pos->next != head)
            {
                if(is_pageclosed(pos,pos->next, i))
                {
                    n = pos->next->next;
                    list_del(pos->next);
                    list_del(pos);
                    queue->nr_free_pages -= 2;
                    page = list_entry(pos, Page, list);
                    list_add(&(page->list),&((queue+1)->list));
                    ++(queue+1)->nr_free_pages;
                }
                else
                {
                }
            }
            pos = n;
        }
    }

    return 0;
}

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
