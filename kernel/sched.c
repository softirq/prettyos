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
#include "lib.h"
#include "timer.h"
#include "kstat.h"
#include "asm-i386/system.h"

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

int goodness(PROCESS **p)
{
    int greatest_ticks = 0;	
    PROCESS *item;
    for(item = proc_table;item < proc_table + NR_PROCESS + NR_PROCS;item++)	
    {
        if(item->flags == FREE_SLOT)
            continue;
        if(item->state != TASK_RUNNING)
        {
            continue;
        }
        else
        {
            //	disp_str(item->name);
            if(item->ticks > greatest_ticks)
            {
                greatest_ticks = item->ticks;
                *p = item;	
            }	
        }
    }
    return greatest_ticks;
}

void switch_to(PROCESS *prev,PROCESS *next)
{
}
void schedule()
{
    disable_int();
    PROCESS*        p;
    //	PROCESS 	*prev,*next;
    int             greatest_ticks = 0;
    //based on PRI
    while (!greatest_ticks) 
    {
        //choose the best process
        greatest_ticks = goodness(&p);
        if(p == current)
        {
            //	disp_str("no best process\n");
        }
        else
        {
            current = p;
        }
        // if all processes execuse over ,asign time to each one,then choose a process
        if (!greatest_ticks) 
        {
            for (p=proc_table; p < proc_table + NR_PROCESS + NR_PROCS; p++) 
            {
                if(p->flags != FREE_SLOT)
                    p->ticks = p->priority;
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
