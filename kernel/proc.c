#include "type.h"
#include "const.h"
#include "list.h"
#include "wait.h"
#include "traps.h"
#include "tty.h"
#include "sched.h"
#include "proc.h"
#include "kernel.h"
#include "sys.h"
#include "printf.h"
#include "clock.h"
#include "pid.h"
#include "exit.h"
#include "fork.h"
#include "stdlib.h"

t8 gdt_ptr[6]; // 0~15:Limit  16~47:Base
DESCRIPTOR  gdt[GDT_SIZE];
t8 idt_ptr[6]; // 0~15:Limit  16~47:Base
GATE idt[IDT_SIZE];

void init_p()
{
    while(1)
    {
        printk("init=%d.",getpid());
        milli_delay(1000);
    }
}


void TestA()
{
    while(1)
    {
        printk("A:%d",getpid() );
        milli_delay(1000);
        /*exit();*/
    }
}


void TestB()
{
    while(1)
    {
        printk("B:%d",getpid() );
        milli_delay(1000);
        /*exit();*/
    }
}

void TestC()
{
    while(1){
        printk("C:%d",getpid() );
        milli_delay(1000);
        fork();
        /*exit();*/
    }
}

void TestD()
{
    while(1)
    {
        printk("D:%d",getpid() );
        milli_delay(1000);
    }
}

void ChildProc()
{
    while(1)
    {
        printk("S:%d",getpid() );
        milli_delay(10);
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

//public	PROCESS	proc_table[NR_TASKS + NR_NATIVE_PROCS];
struct task_struct proc_table[NR_PROCS];

char	task_stack[STACK_SIZE_TOTAL];

//system process tables
TASK	\
            task_table[NR_SYSTEM_PROCS] = { 
                {init_p,   "init"},
                {task_tty, "tty"}
            };

//user process tables;only for test now
TASK 	\
            user_proc_table[NR_USER_PROCS] = { 
                {TestA, "TestA"},
                {TestB, "TestB"},
                {TestC, "TestC"},
                {TestD, "TestD"}
            };


