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
#include "timer.h"
#include "kstat.h"
#include "asm-i386/system.h"

inline void add_wait_queue(struct wait_queue** wq, struct wait_queue *wait)
{
    if(*wq)
    {   
        wait->next = NULL;  
        *wq = wait;
    }   
    else
    {   
        wait->next = (*wq)->next;
        (*wq)->next = wait;
    }   
    return;
}

inline void remove_wait_queue(struct wait_queue **wq, struct wait_queue *wait)
{
    struct wait_queue *tmp;
    if(*wq == wait && ((*wq = wait->next) == wait))
    {   
        *wq = NULL;
    }   
    else
    {   
        tmp = wait;
        while(tmp->next != wait )
        {   
            tmp = tmp->next;
        }   
        tmp->next = wait->next;
    }   
    wait->next = NULL;
    return;
}

void wake_up(struct  wait_queue **wq)
{
    struct wait_queue *tmp;
    struct task_struct *task;

    if(!wq || !(tmp = *wq))
        return;
    do{
        if((task = tmp->task) != NULL)
        {
            if((task->state == TASK_INTERRUPTIBLE) || (task->state == TASK_UNINTERRUPTIBLE))
            {
                task->state = TASK_RUNNING;
                //								if(task->counter > current->counter +3)
                //										need_resched = 1;
            }
        }
        if(!tmp->next)
        {
            break;
        }
        tmp = tmp->next;
    }while(tmp != *wq);
    return;
}

void wake_up_interruptible(struct wait_queue **wq)
{
    struct wait_queue *tmp;
    struct task_struct *task;

    if(!wq || !(tmp = *wq))
        return;
    do{
        if((task = tmp->task) != NULL)
        {
            if(task->state == TASK_INTERRUPTIBLE)
            {
                task->state = TASK_RUNNING;
                //								if(task->counter > current->counter +3)
                //										need_resched = 1;
            }
        }
        if(!tmp->next)
        {
            break;
        }
        tmp = tmp->next;
    }while(tmp != *wq);
    return;
}

static inline void __sleep_on(struct wait_queue **wq, int state)
{
    unsigned long flags;
    struct wait_queue wait = {current, NULL};
    if(!wq)
        return;
    current->state = state;
    save_flags(flags);
    add_wait_queue(wq, &wait);
    schedule();
    remove_wait_queue(wq, &wait);
    restore_flags(flags);
}

void interruptible_sleep_on(struct wait_queue **wq)
{
    __sleep_on(wq, TASK_INTERRUPTIBLE);
}

void sleep_on(struct wait_queue **wq)
{
    __sleep_on(wq, TASK_UNINTERRUPTIBLE);
}

static inline void __down(struct semaphore *sem)
{
    struct wait_queue wait = {current, NULL};
    add_wait_queue(&sem->wait, &wait);
    current->state = TASK_UNINTERRUPTIBLE;
    while(sem->count <= 0)
    {
        schedule();
        current->state = TASK_UNINTERRUPTIBLE;
    }
    current->state = TASK_RUNNING;
    remove_wait_queue(&sem->wait, &wait);
    return;
}

static inline void __up(struct semaphore *sem)
{
    sem->count++;
    wake_up(&sem->wait);
}

void down(struct semaphore *sem)
{
    if(sem->count <= 0)
        __down(sem);
    sem->count--;
}

void up(struct semaphore *sem)
{
    __up(sem);
}
