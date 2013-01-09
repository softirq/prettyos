#include "type.h"
#include "const.h"
#include "traps.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "global.h"
#include "config.h"
#include "kernel.h"
#include "proc.h"
#include "stdlib.h"
#include "linux/net.h"
#include "sched_fair.h"
#include "printf.h"
#include "fork.h"

/*#define EXT_MEM_K 	(*(unsigned short*)0x8002)*/
//long memory_start = 0;
unsigned long buffer_memory_start = 0;
unsigned long buffer_memory_end = 0;
unsigned long main_memory_end = 0;
unsigned long main_memory_start = 0;
unsigned long memory_size = 0;

//init process pid = 0;
//
//get memory size
static int get_memsize(unsigned long *mem_size)
{
    int magic = *(int *)BOOT_PARAM_ADDR;
    /*printk("magic = %x\n", magic);*/
    assert(magic == BOOT_PARAM_MAGIC);

    *mem_size = *(int *)(BOOT_PARAM_ADDR + sizeof(int));
    /*printk("memsize = %x\n", *mem_size);*/

    return 0;
}

static void start_kernel()
{
    init_clock(); //clock interrupt init

    get_memsize(&main_memory_end);
    /*printk("EXT_MEM_K  = %x\n",EXT_MEM_K);*/
    /*main_memory_end = (1<<20) + (EXT_MEM_K << 10);*/
    main_memory_end &= 0xfffff000;
    /*printk("main memory end = %x\n",main_memory_end);*/

    if(main_memory_end > 32 * 1024 * 1024)	//内存大于32M时
    {
        buffer_memory_start = 3 * 1024 * 1024;
        /*buffer_memory_start = 3 * 1024 * 1024 - 512 * 1024;*/
        buffer_memory_end = 4 * 1024 * 1024;
    }
    else 
    {
        buffer_memory_start = 3 * 1024 * 1024;
        buffer_memory_end = 4 * 1024 * 1024;
    }

    main_memory_start = buffer_memory_end;		//主内存的起始地址 = 缓冲区末端
    /*main_memory_start &= 0xfffff000;*/

    buffer_memory_start = ALIGN(buffer_memory_start + BUFFER_SIZE , BUFFER_ALIGN);  //align BUFFER_SIZE
    buffer_memory_end = ALIGN(buffer_memory_end, BUFFER_ALIGN);  //align BUFFER_SIZE
    /*printk("start memroy = %x\t end memory = %x\n",buffer_memory_start,buffer_memory_end);*/
    init_buffer(buffer_memory_start,buffer_memory_end); //buffer init
    /*printk("main memroy start = %x\t main memory end = %x\n",main_memory_start ,main_memory_end);*/
    paging_init();

    init_mem(); //memeory management init
    /* scheduler init */
    init_sched(); 

    /*init_hd(); //hard disk init*/

    /*init_fs(); //filesystem init*/

    /*init_sock();*/

}

/*static struct task_struct g_task[6];*/
/*-----------------------------------------------------------------------------
 *  init the first process
 *-----------------------------------------------------------------------------*/
static void init_task()
{
    int ret;

    disp_str("\tpretty initialize begin\n\n\n\n\n\n\n");

    TASK* p_task;
    PROCESS* p_proc	= proc_table;
    struct task_struct *tsk = NULL;
    char*	p_task_stack = task_stack + STACK_SIZE_TOTAL;
    t16	selector_ldt	= SELECTOR_LDT_FIRST;
    int i,j;

    int prio;
    t8 	privilege;
    t8 	rpl;
    int eflags;

    /*disp_str("\t\tprocess init begins\n");*/
    for(i=0;i<NR_PROCESS + NR_PROCS;++i,++p_proc)
    {
        if(i >= NR_SYSTEM_PROCS + NR_USER_PROCS)
        {
            p_proc->flags = FREE_SLOT;
            continue;
        }

        if(i < NR_SYSTEM_PROCS)
        {
            p_task = task_table + i;
            privilege = PRIVILEGE_TASK;
            rpl = RPL_TASK;
            // IF=1, IOPL=1, bit 2 is always 1.
            eflags = 0x1202;
            prio = 25;
        }
        else if(i < NR_SYSTEM_PROCS + NR_USER_PROCS)
        {
            p_task = user_proc_table + i - NR_SYSTEM_PROCS;
            //	privilege = PRIVILEGE_TASK;
            privilege = PRIVILEGE_USER;
            rpl = RPL_USER;
            // IF=1, IOPL=0, bit 2 is always 1. IO is blocked
            eflags = 0x1202;
            prio = 5;
        }

        tsk = (struct task_struct *)kmem_get_obj(tsk_cachep);

        tsk->state = TASK_RUNNING;
        ret = strcpy(tsk->name, p_task->name);	// name of the process
        if((tsk->pid = get_pidmap()) < 0)
            return;
        tsk->parent = init;
        tsk->next = tsk->sibling = NULL;

        proc_table[0].nr_tty = 0;		// tty 
        for(j = 0; j < NR_SIGNALS;j++)
        {
            tsk->sig_action[j].sa_flags = 0;
            tsk->sig_action[j].sa_handler = do_signal;
        }
        tsk->signal = 0x0; //设置信号为空
        tsk->ldt_sel	= selector_ldt;

        if(strncmp(tsk->name,"init",strlen("init")) != 0)
        {
            p_proc->ldts[INDEX_LDT_C] = gdt[SELECTOR_KERNEL_CS >> 3];
            tsk->ldts[INDEX_LDT_C] = gdt[SELECTOR_KERNEL_CS >> 3];
            p_proc->ldts[INDEX_LDT_C].attr1 = DA_C | privilege << 5;// change the DPL
            tsk->ldts[INDEX_LDT_C].attr1 = DA_C | privilege << 5;// change the DPL
            p_proc->ldts[INDEX_LDT_D] = gdt[SELECTOR_KERNEL_DS >> 3];
            tsk->ldts[INDEX_LDT_D] = gdt[SELECTOR_KERNEL_DS >> 3];
            p_proc->ldts[INDEX_LDT_D].attr1 = DA_DRW | privilege<< 5;// change the DPL
            tsk->ldts[INDEX_LDT_D].attr1 = DA_DRW | privilege<< 5;// change the DPL
        }
        else
        {
            unsigned int k_base;
            unsigned int k_limit;
            int ret = get_kernel_map(&k_base,&k_limit);
            assert(ret == 0);

            init_descriptor(&p_proc->ldts[INDEX_LDT_C],0,(k_base + k_limit) >> LIMIT_4K_SHIFT,DA_32 | DA_LIMIT_4K | DA_C| privilege <<5);
            init_descriptor(&p_proc->ldts[INDEX_LDT_D],0,(k_base + k_limit) >> LIMIT_4K_SHIFT,DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5);

            init_descriptor(&tsk->ldts[INDEX_LDT_C],0,(k_base + k_limit) >> LIMIT_4K_SHIFT,DA_32 | DA_LIMIT_4K | DA_C| privilege <<5);
            init_descriptor(&tsk->ldts[INDEX_LDT_D],0,(k_base + k_limit) >> LIMIT_4K_SHIFT,DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5);
        }

        tsk->regs.cs		= (unsigned int)(((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.ds		= (unsigned int)(((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.es		= (unsigned int)(((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.fs		= (unsigned int)(((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.ss		= (unsigned int)(((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.gs		= (unsigned int)((SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl);

        tsk->regs.eip = (t32)p_task->initial_eip;

        tsk->regs.esp	= (t32)p_task_stack;
        tsk->regs.eflags	= eflags;	
        tsk->ticks = tsk->priority = prio;

        p_task_stack -= p_task->stacksize;
        p_task++;
        selector_ldt += 1 << 3;
        tsk->sched_entity.vruntime = i;
        tsk->sched_class = &rr_sched;
        tsk->sched_class->enqueue_task(&(sched_rq),tsk,0,0);
    }

    k_reenter	= 0;
    ticks		= 0;

    /*current 	= proc_table;*/
}

/* choose a task and begin to run */
static void run_task()
{
    struct rb_root *root = &(cfs_runqueue.task_timeline);
    struct sched_entity *se = ts_leftmost(root);
    current = se_entry(se, struct task_struct, sched_entity);

    move_to_user_mode();
}

/* the kernel main func */
int pretty_main()
{
    start_kernel();
    init_task();
    run_task();

    /* from kernel mode to user mode and scheduler process */
    return 0;
}
