#include "type.h"
#include "const.h"
#include "traps.h"
#include "tty.h"
#include "wait.h"
#include "mm.h"
#include "console.h"
#include "sched.h"
#include "global.h"
#include "kernel.h"
//#include "string.h"
//#include "lib.h"
#include "proc.h"
#include "pgtable.h"

int release_process(struct task_struct *p)
{
    //	free_page_tables(get_base(&(p->ldts[0])),get_limit(&(p->ldts[0])));
    //	free_page_tables(get_base(&(p->ldts[1])),get_limit(&(p->ldts[1])));
    p = NULL;
    //	schedule();
    return 0;
}

static void tell_father(pid)
{
    int i = 0;
    if(pid)
    {
        for(i = 0;i < NR_PROCESS + NR_PROCS;i++)
        {
            /*				if(!proc_table[i])
                            continue;
                            */				if(proc_table[i].pid != pid)
            continue;
            //send signal to father;
            release_process(&proc_table[i]); //it is wrong,must free by father;
            return ;
        }
        printk("BAD BAD - no father found\n\r");
    }
    release_process(current);
}

int do_exit()
{
    int i;
    struct task_struct *p = current;
    int pid = p->pid;	
    printk("\nexit process pid = %d\n",pid);
    for(i = 0;i < NR_PROCESS + NR_PROCS;i++)
    {
        if(proc_table[i].parent == pid)
        {
            proc_table[i].parent = 1; //init pid
            if(proc_table[i].state == TASK_WAITING && proc_table[i].state == TASK_ZOMBIE)
            {
                //			release_process(&proc_table[i]);
            }
        }
    }
    p->state = TASK_ZOMBIE;
    //	p->exit_code = code;
    tell_father(p->parent);
    schedule();
    return 0;
}

int sys_exit()
{
    return do_exit();
}
