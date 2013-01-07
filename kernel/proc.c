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
#include "proc.h"
#include "kernel.h"
#include "stdlib.h"
#include "sys.h"
#include "fork.h"
#include "exit.h"
#include "printf.h"

void init_p()
{
    //	disp_str("init() is running ...\n");
    pid_t pid = 0;

    /*pid = fork();*/
    if(pid != 0)
    {
        printk("parnet is running,parent pid = %d \n",getpid());
    }
    else
    {
        printk("child is running,child pid  = %d\n",getpid());
    }
    //	current->state = TASK_WAITING;
    while(1)
    {
    }
}


void TestA()
{
    while(1)
    {
        //syscall sys_get_ticks
        //		disp_int(get_ticks());
        disp_str("A");
        //		printf("sunkang A:%s",get_ticks());
        milli_delay(100);
    }
}


void TestB()
{
    //	int i = 0;
    while(1)
    {
        disp_str("B");
        //	printf("%d %s %c\t",i,"sunkang",'k');
        //		i++;
        //		assert(i ==1);
        //		panic("no error\n");
        //		TestA();
        milli_delay(100);
        /*exit();*/
    }
}

void TestC()
{
    //	int i = 0;
    while(1){
        disp_str("C");
        //		waitpid(current->pid);
        milli_delay(100);
    }
}

void TestD()
{
    while(1)
    {
        disp_str("D");
        milli_delay(100);
    }
}
//get ldt segment address
int ldt_seg_linear(struct task_struct *p,int idx)
{
    struct descriptor *d = &(p->ldts[idx]);
    return (d->base_low | d->base_mid << 16 | d->base_high << 24);
}

//virtual address to liner address
void* va2la(int pid,void *va)
{
    struct task_struct *p = &proc_table[pid];
    u32 seg_base = ldt_seg_linear(p,INDEX_LDT_D);	
    u32 la = seg_base + (u32)va;
    return (void *)la;
}
