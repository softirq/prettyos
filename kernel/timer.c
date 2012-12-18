#include "type.h"
#include "const.h"
#include "traps.h"
//#include "string.h"
#include "tty.h"
#include "console.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "global.h"
#include "kernel.h"
#include "stdlib.h"
#include "linux/timer.h"
#include "printf.h"

static struct timer_list timer_header = 
{
    &timer_header,
    &timer_header,
    ~0,
    NULL

};

void do_gettimeofday(struct timeval *tv)
{
    tv->tv_sec = 0;
    tv->tv_usec = 0;
    return;
}

int init_timer(struct timer_list *timer)
{
    timer->prev = NULL;
    timer->next = NULL;
    return 0;
}
int add_timer(struct timer_list *timer)
{
    struct timer_list *p;
    struct timer_list *q;
    timer->expires += jiffies;

    p = &timer_header;
    do
    {
        q  = p;
        p = p->next;
    }while(p->expires < timer->expires);
    if(p)
    {
        timer->next = p;
        timer->prev = p->prev;
        p->prev = timer;
        timer->prev->next = timer;
    }
    else
    {
        q->next = timer;
        timer->prev = q;
        timer->next = NULL;
    }
    return 0;
}

int del_timer(struct timer_list *timer)
{
    struct timer_list *p = &timer_header;

    while((p = p->next))
    {
        if(p == timer)
        {
            p->prev->next = p->next;
            p->next->prev = p->prev;
            timer->prev = timer->next = NULL;
            timer->expires -= jiffies;
            return 0;
        }
    }
    if(timer->prev || timer->next)
        printk("no such timer in timer_lists\n");
    return -1;
}
