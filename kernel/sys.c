#include "type.h"
#include "const.h"
#include "list.h"
#include "wait.h"
#include "string.h"
#include "tty.h"
#include "fs.h"
#include "exit.h"
#include "fork.h"
#include "clock.h"
#include "printf.h"
#include "sys.h"

int sys_get_ticks()
{
    return ticks;
}

int sys_waitpid(int pid)
{
    /*struct task_struct *p;*/
    /*int i;*/
#if 0
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
#endif
    return 0;
}

int sys_wakeup()
{
    return 0;
}

//system call table
syscall_ptr		\
                    sys_call_table[NR_SYS_CALL] = {
                        sys_get_ticks,
                        sys_write,
                        sys_printx,
                        sys_fork,
                        sys_exit,
                        sys_waitpid
                    };
