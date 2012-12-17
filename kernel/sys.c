#include "type.h"
#include "const.h"
#include "traps.h"
#include "string.h"
#include "tty.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "console.h"
#include "proc.h"
#include "global.h"
#include "kernel.h"
#include "errno.h"

int sys_get_ticks()
{
    return ticks;
}

int sys_waitpid(int pid)
{
    /*struct task_struct *p;*/
    int i;
    for(i = 0;i < NR_PROCESS + NR_PROCS;i++)
    {
        if(!pid)
            break;
        else if(pid > 0)
        {
            if(proc_table[i].pid != pid)
            {
                continue;
                //	p->pid = TASK_WAITING;
            }
        }
        /*		else if(pid == -1)
                {
                if(proc_table[i].parent != pid)
                continue;
                }
                else if(pid < -1)
                {
                continue;
                }
                switch(p->state)
                {
                case TASK_STOPPED:
                return p->pid;
                case TASK_ZOMBIE:
                release_process(p);
                return 

                }
                */
        /*p->state = TASK_UNINTERRUPTIBLE;*/
        schedule();
        break;
    }
    return 0;
}

int sys_wakeup()
{
    return 0;
}
