#include "type.h"
#include "const.h"
#include "string.h"
#include "stdlib.h"
#include "list.h"
#include "panic.h"
#include "wait.h"
#include "mm.h"
#include "printf.h"
#include "list.h"
#include "math.h"

static int count = 0;
struct buddy_list buddy_list[NR_MEM_LISTS];

/* print buddy list info */
int print_buddy_list()
{
    int i = 0;
    struct buddy_list *queue = NULL;

    printk("\n---------------------------\n");
    for(i = 0;i < NR_MEM_LISTS; i++)
    {
        queue = buddy_list + i;
        printk("2 i = %d nr_free_pages = %d.\n", i, queue->nr_free_pages);
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
        return -1;
    }

    struct buddy_list *header = buddy_list + order;
    struct list_head *head, *pos, *n;
    struct page *item_page = NULL;

    head = &(header->list);
    list_for_each_safe(pos, n, head)
    {
        /*list_add(&(page->list),&(header->list));*/
        item_page = list_entry(pos, Page, list);
        if(item_page->address < page->address)
            continue;
        else
        {
            __list_add(&(page->list), pos->prev, pos);
            ++buddy_list[order].nr_free_pages;
            return 0;
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
        count = 0;
        queue = buddy_list + i;
        printk("1 i = %d nr_free_pages = %d.\n", i, queue->nr_free_pages);

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
                /*printk("*");*/
                if(is_pageclosed(pos,pos->next, i))
                {
                    ++count;
                    /*printk("#");*/
                    n = pos->next->next;
                    list_del(pos->next);
                    list_del(pos);
                    queue->nr_free_pages -= 2;
                    /*printk("@");*/
                    page = list_entry(pos, Page, list);
                    list_add(&(page->list),&((queue+1)->list));
                    /*printk("&");*/
                    ++(queue+1)->nr_free_pages;
                }
                else
                {
                    /*printk("#");*/
                }
            }
            pos = n;
        }
        printk("count=%d\n", count);
    }

    return 0;
}
