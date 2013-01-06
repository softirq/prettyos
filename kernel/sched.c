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
#include "proc.h"
#include "kernel.h"
#include "stdlib.h"
#include "timer.h"
#include "kstat.h"
#include "printf.h"
#include "asm-i386/system.h"
#include "asm-i386/panic.h"

struct task_struct *current = NULL;
struct list_head run_queue;
struct task_struct *init = &proc_table[1];
struct kernel_stat kstat = { 0 };

void unblock(struct task_struct *p)
{
    p->state = TASK_RUNNING;
}

void block(struct task_struct *p)
{
    p->state = TASK_INTERRUPTIBLE;
    schedule();
}

int goodness(struct task_struct **p)
{
    int greatest_ticks = 0;	
    /*struct task_struct *iter = NULL;*/
    struct list_head *head, *pos, *n;

    head = &(run_queue);

    if(list_empty_careful(head))
    {
        panic("no task in running queue.");
    }

    list_for_each_safe(pos, n, head)
    {
        struct task_struct *tsk = list_entry(pos, struct task_struct, list);
        assert(tsk->state == TASK_RUNNING);
        if(tsk->ticks > greatest_ticks)
        {
            greatest_ticks = tsk->ticks;
            *p = tsk ;	
        }	
    }

    /*disp_int(greatest_ticks);*/
    return greatest_ticks;
}

void switch_to(PROCESS *prev,PROCESS *next)
{
}

void schedule()
{
    disable_int();
    struct task_struct *p = NULL, *tsk = NULL;
    struct list_head *head, *pos, *n;
    int greatest_ticks = 0;

    //based on PRI
    while (!greatest_ticks) 
    {
        //choose the best process
        greatest_ticks = goodness(&p);

        if(p)
        {
            if(p == current)
            {
            }
            else
            {
                current = p;
            }
        }

        // if all processes execuse over ,asign time to each one,then choose a process
        if (!greatest_ticks) 
        {
            head = &(run_queue);
            list_for_each_safe(pos, n, head)
            {
                tsk = list_entry(pos, struct task_struct, list);
                tsk->ticks = tsk->priority;
            }
        }
    }

    enable_int();
}

/*插入到running queue*/
int insert_rq(struct task_struct *p)
{
    if(p)
    {
        list_add(&(p->list), &run_queue);
        return 0;
    }

    return -1;
}

int delete_rq(struct task_struct *p)
{
    if(p)
    {
        list_del(&(p->list));
        return 0;
    }

    return -1;
}
