#include "type.h"
#include "const.h"
#include "traps.h"
#include "tty.h"
#include "console.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "stddef.h"
#include "bitmap.h"
#include "global.h"
#include "kernel.h"
#include "string.h"
#include "stdlib.h"
#include "panic.h"
#include "proc.h"
#include "printf.h"
#include "fork.h"

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

int getpid()
{
    return current->pid;
}

void init_pidmap()
{
    bzero((void *)pidmap,sizeof(pidmap)); 
}

pid_t get_pidmap()
{
    pid_t first = (pid_t)set_first_bit(pidmap,ARRAY_SIZE(pidmap));
    return first;
}

static struct task_struct * get_empty_process(void)
{
    struct task_struct *tsk = (struct task_struct *)kmem_get_obj(tsk_cachep);
    return tsk;
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
}

//copy the parent process address space
static int copy_mem(struct task_struct *p)
{
    struct descriptor *dp = &current->ldts[INDEX_LDT_C];
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

    /*unsigned long child_base = alloc_mem(text_size);*/

    //	printk("child_base = %d\t text_base = %d\t text_size = %d\n",child_base,text_base,text_size);
    /*memcpy((void *)child_base,(void *)(text_base),text_size);*/

    //	printk("child_base = %d\t text_base = %d\t text_size = %d\n",child_base,text_base,text_size);
    /*phys_copy((char *)child_base,(char *)(text_base),text_size);*/

    unsigned int k_base;
    unsigned int k_limit;
    int ret = get_kernel_map(&k_base,&k_limit);
    assert(ret == 0); 
    unsigned char privilege = PRIVILEGE_USER;;

    init_descriptor(&p->ldts[INDEX_LDT_C],0,(k_base + k_limit) >> LIMIT_4K_SHIFT,DA_32 | DA_LIMIT_4K | DA_C| privilege <<5);
    init_descriptor(&p->ldts[INDEX_LDT_D],0,(k_base + k_limit) >> LIMIT_4K_SHIFT,DA_32 | DA_LIMIT_4K | DA_DRW | privilege <<5);            

    /*init_descriptor(&p->ldts[INDEX_LDT_C],child_base,(PROC_IMAGE_SIZE_DEFAULT - 1)>>LIMIT_4K_SHIFT,DA_LIMIT_4K | DA_32 |DA_C|PRIVILEGE_USER << 5);*/
    /*init_descriptor(&p->ldts[INDEX_LDT_D],child_base,(PROC_IMAGE_SIZE_DEFAULT - 1)>>LIMIT_4K_SHIFT,DA_LIMIT_4K | DA_32 |DA_DRW|PRIVILEGE_USER << 5);*/

    return 0;
}

int do_fork()
{
    struct task_struct *p; 

    p = get_empty_process();
    if(p == NULL)
        panic("cannot get empty task_struct struct \n");

    memcpy(p,current,sizeof(struct task_struct));

    p->state = TASK_UNINTERRUPTIBLE;

    if((p->pid = get_pidmap()) < 0)
        return -1;

    p->parent = current;
    /*p->regs.eflags = 0x1202;*/

    sprintf(p->name,"%s",current->name);

    /*copy_mem(p);*/

    p->state = TASK_RUNNING;

    p->sched_entity.vruntime = 10;
    /*p->sched_class = &rr_sched;*/
    p->sched_class->enqueue_task(&(sched_rq),p,0,0);

    /*printk("fork name = %s.\n",p->name);*/
    /*printk("eip = %x.\n",p->regs.eip);*/
    /*printk("eip = %x.\n",&p->regs.eip);*/
    /*copy_regs(p);*/
    p->regs.eip = (unsigned int)ChildProc;

    return p->pid;
}

int sys_fork()
{
    //	int flag;
    return do_fork();
}
