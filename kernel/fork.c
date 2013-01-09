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

int get_bitmap()
{
    int first = set_first_bit(pidmap,ARRAY_SIZE(pidmap));
    return first;
}

static struct task_struct * get_empty_process(void)
{
    struct task_struct *tsk = (struct task_struct *)kmem_get_obj(tsk_cachep);
    return tsk;
}

////复制父进程的地址空间
static int copy_mem(int pid,struct task_struct *p)
{
#if 0
    struct descriptor *dp = &proc_table[pid].ldts[INDEX_LDT_C];
    int text_base = get_base(dp);
    int text_limit = get_limit(dp);
    int text_size = (text_limit + 1) * ((dp->limit_high_attr2 * 0x80)?4096:1);

    dp = &proc_table[pid].ldts[INDEX_LDT_D]; 
    int data_base =  get_base(dp);
    int data_limit= get_limit(dp);
    int data_size = (text_limit + 1) * ((dp->limit_high_attr2 * 0x80)?4096:1);

    assert((text_base == data_base)  && 
            (text_limit == data_limit) &&
            (text_size == data_size)
          );

    /*int child_base = alloc_mem(p->pid,text_size);*/

    //	printk("child_base = %d\t text_base = %d\t text_size = %d\n",child_base,text_base,text_size);
    //	memcpy((void *)child_base,(void *)(text_base),text_size);

    //	printk("child_base = %d\t text_base = %d\t text_size = %d\n",child_base,text_base,text_size);
    /*phys_copy((char *)child_base,(char *)(text_base),text_size);*/
    return child_base;
#endif
    return 0;
}

int do_fork()
{
    /*int ret = -1;*/
    /*int child_pid = -1; 	//child process pid*/
    int pid = current->pid; //parent process pid;
    int child_base = 0;
    struct task_struct *p; 
    //	struct file *f;
    //	p = (struct task_struct *)get_free_page();
    p = get_empty_process();

    if(p == NULL)
        panic("cannot get empty task_struct struct \n");


    p->state = TASK_UNINTERRUPTIBLE;
    /*p->pid = child_pid;*/
    p->parent = current;
    p->regs.eflags = 0x1202;
    /*sprintf(p->name,"%s-%d",p->name,p->pid);*/
    //	printk("p->name= %s\n",p->name);
    child_base = copy_mem(pid,p);
    /*
       for(i = 0;i < NR_OPEN;i++)
       {
       if((f = p->filp[i]) != NULL)	
       {
       f->f_count++;
       }
       }
       if(current->pwd)
       current->pwd->i_count++;
       if(current->root)
       current->root->i_count++;
       */	
    init_descriptor(&p->ldts[INDEX_LDT_C],child_base,(PROC_IMAGE_SIZE_DEFAULT - 1)>>LIMIT_4K_SHIFT,DA_LIMIT_4K | DA_32 |DA_C|PRIVILEGE_USER << 5);
    init_descriptor(&p->ldts[INDEX_LDT_D],child_base,(PROC_IMAGE_SIZE_DEFAULT - 1)>>LIMIT_4K_SHIFT,DA_LIMIT_4K | DA_32 |DA_DRW|PRIVILEGE_USER << 5);
    p->state = TASK_RUNNING;
    return pid;
}

int sys_fork()
{
    //	int flag;
    return do_fork();
}
