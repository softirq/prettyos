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
#include "stdlib.h"

t8      gdt_ptr[6]; // 0~15:Limit  16~47:Base
DESCRIPTOR  gdt[GDT_SIZE];
t8      idt_ptr[6]; // 0~15:Limit  16~47:Base
GATE        idt[IDT_SIZE];


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
        /*fork();*/
        /*exit();*/
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

void ChildProc()
{
    while(1)
    {
        disp_str("Child");
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

//public	PROCESS	proc_table[NR_TASKS + NR_NATIVE_PROCS];
struct task_struct proc_table[NR_PROCESS + NR_PROCS];

char	task_stack[STACK_SIZE_TOTAL];

//system process tables
TASK	\
            task_table[NR_SYSTEM_PROCS] = { 
                {task_tty, STACK_SIZE_TTY, "tty"}
            };

//user process tables;only for test now
TASK 	\
            user_proc_table[NR_USER_PROCS] = { 
                {init_p, 	STACK_SIZE_INIT,   "init"},
                {TestA, STACK_SIZE_TESTA, "TestA"},
                {TestB, STACK_SIZE_TESTB, "TestB"},
                {TestC, STACK_SIZE_TESTC, "TestC"},
                {TestD, STACK_SIZE_TESTD, "TestD"}
            };


