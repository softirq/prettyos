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
#include "printf.h"

int release_process(struct task_struct *p)
{
    //	free_page_tables(get_base(&(p->ldts[0])),get_limit(&(p->ldts[0])));
    //	free_page_tables(get_base(&(p->ldts[1])),get_limit(&(p->ldts[1])));
    p = NULL;
    //	schedule();
    return 0;
}

#if 0
static void tell_father(pid)
{
    struct task_struct *iter = NULL;
    /*int i = 0;*/
    if(pid)
    {
        for(iter = run_queue; iter; iter = iter->next)
        {
            if(iter->pid != pid)
                continue;
            //send signal to father;
            release_process(iter); //it is wrong,must free by father;
            return ;
        }
        printk("BAD BAD - no father found\n\r");
    }
    /*release_process(current);*/
}
#endif

int do_exit()
{
    struct task_struct *p = current, *tsk = NULL;
    int pid = p->pid;	
    printk("\nexit process pid = %d\n",pid);

    struct list_head *head, *pos, *n;
    /*head = &(run_queue);*/
    head = &(rr_runqueue.rr_rq_list);
    list_for_each_safe(pos, n, head)
    {
        tsk = list_entry(pos, struct task_struct, list);

        if(tsk->parent->pid == pid)
        {
            /*parent is init task*/
            tsk->parent = init;
            if(tsk->state == TASK_WAITING && tsk->state == TASK_ZOMBIE)
            {
                //			release_process(&proc_table[i]);
            }
        }
    }

    p->state = TASK_ZOMBIE;

    /*delete_rq(p);*/
    p->sched_class->dequeue_task(&(sched_rq),p,0);
    
    //	p->exit_code = code;
    /*tell_father(p->parent);*/
    schedule();
    return 0;
}

int sys_exit()
{
    return do_exit();
}
