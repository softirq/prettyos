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
struct task_struct *init = &proc_table[1];
struct kernel_stat kstat = { 0 };

struct rq sched_rq;

void init_rq()
{
    rr_runqueue.rr_nr_running = 0;
    INIT_LIST_HEAD(&(rr_runqueue.rr_rq_list));
}

void unblock(struct task_struct *p)
{
    p->state = TASK_RUNNING;
}

void block(struct task_struct *p)
{
    p->state = TASK_INTERRUPTIBLE;
    schedule();
}

struct task_struct * rr_next_task(struct rq *rq)
{
    struct task_struct *p = NULL, *tsk = NULL;
    struct list_head *head, *pos, *n;
    int greatest_ticks = 0;	

rr_repeat:
    head = &(rq->u.rr.rr_rq_list);

    if(list_empty_careful(head))
    {
        panic("no task in running queue.");
    }

    list_for_each_safe(pos, n, head)
    {
        tsk = list_entry(pos, struct task_struct, list);
        assert(tsk->state == TASK_RUNNING);
        if(tsk->ticks > greatest_ticks)
        {
            greatest_ticks = tsk->ticks;
            p = tsk ;	
        }	
    }

    if (!greatest_ticks) 
    {
        head = &(rq->u.rr.rr_rq_list);
        list_for_each_safe(pos, n, head)
        {
            tsk = list_entry(pos, struct task_struct, list);
            tsk->ticks = tsk->priority;
        }
        goto rr_repeat;
    }

    /*disp_int(greatest_ticks);*/
    return p;
}

#if 0
int goodness(struct task_struct **p)
{
    int greatest_ticks = 0;	
    /*struct task_struct *iter = NULL;*/
    struct list_head *head, *pos, *n;

    head = &(fifo_queue);

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
#endif

void switch_to(PROCESS *prev,PROCESS *next)
{
}

/*插入到running queue*/
void rr_enqueue(struct rq *rq, struct task_struct *p, int wakeup,bool head)
{
    if(p)
    {
        struct list_head *head = &(rq->u.rr.rr_rq_list);
        list_add(&(p->list), head);
    }
}

#if 0
int insert_rq(struct task_struct *p)
{
    if(p)
    {
        list_add(&(p->list), &fifo_queue);
        return 0;
    }

    return -1;
}
#endif

void rr_dequeue(struct rq *rq, struct task_struct *p, int sleep)
{
    if(p)
    {
        /*struct list_head *head = &(rq->u.rr.rr_rq_list);*/
        list_del(&(p->list));
    }
}

#if 0
int delete_rq(struct task_struct *p)
{
    if(p)
    {
        list_del(&(p->list));
        return 0;
    }

    return -1;
}
#endif

struct sched_class rr_sched = 
{
    .enqueue_task = rr_enqueue,
    .dequeue_task = rr_dequeue,
    .pick_next_task = rr_next_task,
    .switched_from = NULL,
    .switched_to = NULL,
    .prio_changed = NULL,
};

struct sched_class cfs_sched = 
{
    .enqueue_task = NULL,
    .dequeue_task = NULL,
    .pick_next_task = NULL,
    .switched_from = NULL,
    .switched_to = NULL,
    .prio_changed = NULL,
};

void schedule()
{
    disable_int();

    struct task_struct *p = NULL;

    //based on PRI
        //choose the best process
    /*greatest_ticks = goodness(&p);*/
    p = current->sched_class->pick_next_task(&sched_rq); 

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

    enable_int();
}
