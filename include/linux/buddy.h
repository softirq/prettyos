#ifndef     _BUDDY_H_
#define     _BUDDY_H_

#include "page.h"

#define NR_MEM_LISTS   10 

struct buddy_list 
{
    struct list_head list;
    int nr_free_pages;
};

extern struct buddy_list buddy_list[NR_MEM_LISTS];

extern int print_buddy_list();
extern int is_pageclosed(struct list_head *prev, struct list_head *next, int order);
extern int buddy_list_add(struct page *page, const int order);
extern int buddy_list_del(struct list_head *head, const int order, struct list_head **item);
extern int buddy_list_add_line(struct page *page, const int order);
extern void buddy_list_init ();
extern int buddy_list_tidy();

extern unsigned long get_free_pages(int order);
extern unsigned long get_free_page(int priority);
extern int free_pages(struct page *page, const int order);
//extern struct page*__get_free_pages(const int priority, const int gfporder);

#define  	__get_free_page(priority) 		__get_free_pages((priority),0)
#define     page_address(page)      (page->address)

#endif
