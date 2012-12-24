#ifndef     _BUDDY_H_
#define     _BUDDY_H_

#include "list.h"

#define NR_MEM_LISTS    6
/*
 * 1 page  1024
 * 2 pages 512
 * 3 pages 256 
 * 4 pages 128 
 * 5 pages 64 
 * 6 pages 32 
 *
 * */

struct buddy_list 
{
    struct list_head list;
    int nr_free_pages;
};

extern struct buddy_list buddy_list[NR_MEM_LISTS];

int print_buddy_list();
int is_pageclosed(struct list_head *prev, struct list_head *next, int order);
int buddy_list_add(struct page *page, const int order);
void buddy_list_init ();
int buddy_list_tidy();

#endif
