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

struct task_struct *current = NULL;
struct task_struct *run_queue = NULL;
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
    struct task_struct *iter = NULL;
    
    for(iter = run_queue;iter; iter = iter->next)	
    {
        assert(iter->state == TASK_RUNNING);
        
        if(iter->ticks > greatest_ticks)
        {
            greatest_ticks = iter->ticks;
            *p = iter;	
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
    struct task_struct *p = NULL, *iter = NULL;
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
                //	disp_str("no best process\n");
            }
            else
            {
                current = p;
            }
        }

        /*disp_str(current->name);*/
        // if all processes execuse over ,asign time to each one,then choose a process
        if (!greatest_ticks) 
        {
            disp_int(4);
            for(iter = run_queue; iter; iter = iter->next)	
            {
                iter->ticks = iter->priority;
            }
        }
        /*		else
                {
                switch_to(prev,next);
                }
                */
    }

    enable_int();
}

/*插入到running queue*/
int insert_rq(struct task_struct *p)
{
    if(p == NULL)
    {
        return -1;
    }

    p->next = run_queue;
    run_queue = p;
    /*disp_str("insert into running queue\n");*/

    return 0;
}

int delete_rq(struct task_struct *p)
{
    struct task_struct *iter = NULL, *front = NULL;

    if(p == NULL)
    {
        return -1;
    }

    for(iter = run_queue; iter; iter = iter->next)
    {
        if(iter->pid == p->pid)
        {
            if(front == NULL)
            {
                run_queue = iter->next;
            }
            else
            {
                front->next = iter->next;
            }

            return 0;
        }
        front = iter;
    }

    return -1;
}
