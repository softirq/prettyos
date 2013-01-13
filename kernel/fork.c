#include "type.h"
#include "const.h"
#include "list.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "string.h"
#include "panic.h"
#include "printf.h"
#include "fork.h"
#include "start.h"

unsigned char pidmap[MAX_PIDNUM/8] = {0};

int get_limit(struct descriptor *dp)
{
    if(dp)
    {
        return ((dp->limit_high_attr2 & 0xF) >> DP_LIMIT_SHIFT) + dp->limit_low;
    }
    return 0;
}

int get_base(struct descriptor *dp)
{
    if(dp)
    {
        return ((dp->base_high << DP_BASE_HIGH_SHIFT) + (dp->base_mid << DP_BASE_MID_SHIFT) + dp->base_low);
    }
    return 0;
}

static struct task_struct * get_empty_process(void)
{
    return (struct task_struct *)kmem_get_obj(tsk_cachep);
}

static void copy_regs(struct task_struct *p)
{
    p->regs.cs = current->regs.cs;
    p->regs.ds = current->regs.ds;
    p->regs.es = current->regs.es;
    p->regs.fs = current->regs.fs;
    p->regs.ss = current->regs.ss;
    p->regs.gs = current->regs.gs;
    p->regs.eip = current->regs.eip;
    p->regs.esp = current->regs.esp;
    p->regs.eflags = current->regs.eflags;
    p->ticks = current->ticks;
    p->priority = current->priority;
}

//copy the parent process address space
static int copy_mem(struct task_struct *p)
{
    /*struct descriptor *dp = &current->ldts[INDEX_LDT_C];
    int text_base = get_base(dp);
    int text_limit = get_limit(dp);
    int text_size = (text_limit + 1) * ((dp->limit_high_attr2 * 0x80)?4096:1);

    dp = &current->ldts[INDEX_LDT_D]; 
    int data_base =  get_base(dp);
    int data_limit= get_limit(dp);
    int data_size = (text_limit + 1) * ((dp->limit_high_attr2 * 0x80)?4096:1);

    assert((text_base == data_base)  && 
            (text_limit == data_limit) &&
            (text_size == data_size)
          );
*/
    /*unsigned long child_base = alloc_mem(text_size);*/

    //	printk("child_base = %d\t text_base = %d\t text_size = %d\n",child_base,text_base,text_size);
    /*memcpy((void *)child_base,(void *)(text_base),text_size);*/

    //	printk("child_base = %d\t text_base = %d\t text_size = %d\n",child_base,text_base,text_size);
    /*phys_copy((char *)child_base,(char *)(text_base),text_size);*/

    /*unsigned int k_base;
    unsigned int k_limit;
    int ret = get_kernel_map(&k_base,&k_limit);
    assert(ret == 0); 
    unsigned char privilege = PRIVILEGE_USER;;

    init_descriptor(&p->ldts[INDEX_LDT_C],0,(k_base + k_limit) >> LIMIT_4K_SHIFT,DA_32 | DA_LIMIT_4K | DA_C| privilege <<5);
    init_descriptor(&p->ldts[INDEX_LDT_D],0,(k_base + k_limit) >> LIMIT_4K_SHIFT,DA_32 | DA_LIMIT_4K | DA_DRW | privilege <<5);            
*/
    memcpy(&p->ldts,&current->ldts, sizeof(current->ldts));

    return 0;
}

int do_fork()
{
    int i = 0;
    struct task_struct *p = get_empty_process();
    if(p == NULL)
        panic("cannot get empty task_struct struct \n");


    if((p->pid = get_pidmap()) < 0)
        return -1;

    p->parent = current;
    p->next = p->sibling = NULL;

    for(i = 0; i < NR_SIGNALS;++i)
    {
        p->sig_action[i].sa_flags = 0;
        p->sig_action[i].sa_handler = do_signal;
    }

    p->signal = 0x0; //设置信号为空

    /* inherit the parent scheduler class */
    p->sched_class = current->sched_class;

    /* share ldt selector with the parent process
     * it import for fork */
    p->ldt_sel = current->ldt_sel;

    copy_mem(p);
    copy_regs(p);

    p->regs.eip = (unsigned int)ChildProc;

    p->sched_entity.vruntime = 10;
    p->state = TASK_RUNNING;
    p->sched_class->enqueue_task(&(sched_rq),p,0,0);

    return p->pid;
}

int sys_fork()
{
    //	int flag;
    return do_fork();
}
