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
/*#include "fork.h"*/
#include "linux/net.h"

#define EXT_MEM_K 	(*(unsigned short*)0x8002)
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
    printk("memsize = %x\n", *mem_size);

    return 0;
}

static void start_kernel()
{
    //	init_trap();// define in kernel/start.c

    disp_str("-------------------------------------\n");

    init_clock(); //clock interrupt init

    get_memsize(&main_memory_end);
    /*printk("EXT_MEM_K  = %x\n",EXT_MEM_K);*/
    /*main_memory_end = (1<<20) + (EXT_MEM_K << 10);*/
    main_memory_end &= 0xfffff000;
    printk("main memory end = %x\n",main_memory_end);

    if(main_memory_end > 12 * 1024 * 1024)	//内存大于6M时
    {
        buffer_memory_start = 3 * 1024 * 1024;
        buffer_memory_end = 4 * 1024 * 1024;
    }
    else 
    {
        buffer_memory_start = 3 * 1024 * 1024 - 512 * 1024;
        buffer_memory_end = 3 * 1024 * 1024;
    }

    main_memory_start = buffer_memory_end;		//主内存的起始地址 = 缓冲区末端
    /*main_memory_start &= 0xfffff000;*/
    /*disp_str("-------------------------------------\n");*/

    /*buffer_memory_end = (buffer_memory_end + BUFFER_SIZE) & (~BUFFER_SIZE)- 1;  //align BUFFER_SIZE*/
    buffer_memory_start = ALIGN(buffer_memory_start + BUFFER_SIZE , BUFFER_ALIGN);  //align BUFFER_SIZE
    buffer_memory_end = ALIGN(buffer_memory_end, BUFFER_ALIGN);  //align BUFFER_SIZE
    printk("start memroy = %x\t end memory = %x\n",buffer_memory_start,buffer_memory_end);
    init_buffer(buffer_memory_start,buffer_memory_end); //buffer init
    printk("main memroy start = %x\t main memory end = %x\n",main_memory_start ,main_memory_end);
    paging_init(main_memory_start,main_memory_end);

    init_mem(main_memory_start,main_memory_end); //memeory management init

    /*init_hd(); //hard disk init*/

    /*init_fs(); //filesystem init*/

    /*init_sock();*/

    move_to_user_mode();

    while(1){}
}

/*-----------------------------------------------------------------------------
 *  init the first process
 *-----------------------------------------------------------------------------*/
static void init_task()
{
    int ret;

    disp_str("\tpretty initialize begin\n");

    TASK*	p_task;
    //		= task_table;
    PROCESS*	p_proc		= proc_table;
    char*	p_task_stack	= task_stack + STACK_SIZE_TOTAL;
    t16	selector_ldt	= SELECTOR_LDT_FIRST;
    int i,j;

    int prio;
    t8 	privilege;
    t8 	rpl;
    int eflags;

    disp_str("\t\tprocess init begins\n");
    for(i=0;i<NR_PROCESS + NR_PROCS;i++,p_proc++)
    {
        if(i >= NR_SYSTEM_PROCS + NR_USER_PROCS)
        {
            //	printk(" i = %d\t ",i);
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

        //		memset(p_proc,0,sizeof(struct task_struct));
        p_proc->state = TASK_RUNNING;
        /*p_proc->flags = 1;*/
        ret = strcpy(p_proc->name, p_task->name);	// name of the process
        p_proc->pid = i;			// pid
        /*disp_str("*****************\n");*/
        /*p_proc->parent = NO_PARENT;*/

        /*proces family */
        p_proc->parent = init;
        p_proc->next = p_proc->sibling = NULL;

        proc_table[0].nr_tty = 0;		// tty 
        for(j = 0; j < NR_SIGNALS;j++)
        {
            p_proc->sig_action[j].sa_flags = 0;
            p_proc->sig_action[j].sa_handler = do_signal;
        }
        //	p_proc->sig_blocked = 0; //设置信号屏蔽字为空
        p_proc->signal = 0x0; //设置信号为空
        p_proc->ldt_sel	= selector_ldt;
        if(strncmp(p_proc->name,"init",strlen("init")) != 0)
        {
            p_proc->ldts[INDEX_LDT_C] = gdt[SELECTOR_KERNEL_CS >> 3];
            //memcpy(&p_proc->ldts[INDEX_LDT_C], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
            p_proc->ldts[INDEX_LDT_C].attr1 = DA_C | privilege << 5;// change the DPL
            //memcpy(&p_proc->ldts[INDEX_LDT_D], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
            p_proc->ldts[INDEX_LDT_D] = gdt[SELECTOR_KERNEL_DS >> 3];
            p_proc->ldts[INDEX_LDT_D].attr1 = DA_DRW | privilege<< 5;// change the DPL
        }
        else
        {
            printk(" wo shi init.............\n");
            unsigned int k_base;
            unsigned int k_limit;
            int ret = get_kernel_map(&k_base,&k_limit);
            printk("k_base = %d k_limit = %d\n",k_base,k_limit);
            printk("k_base = 0x%x k_limit = 0x%x\n",k_base,k_limit);
            assert(ret== 0);
            init_descriptor(&p_proc->ldts[INDEX_LDT_C],0,(k_base + k_limit) >> LIMIT_4K_SHIFT,DA_32 | DA_LIMIT_4K | DA_C| privilege <<5);
            init_descriptor(&p_proc->ldts[INDEX_LDT_D],0,(k_base + k_limit) >> LIMIT_4K_SHIFT,DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5);
        }

        p_proc->regs.cs		= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.ds		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.es		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.fs		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.ss		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.gs		= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

        p_proc->regs.eip	= (t32)p_task->initial_eip;
        /*disp_int(p_proc->regs.eip);*/

        p_proc->regs.esp	= (t32)p_task_stack;
        p_proc->regs.eflags	= eflags;	
        p_proc->ticks = p_proc->priority = prio;

        p_task_stack -= p_task->stacksize;
        p_task++;
        selector_ldt += 1 << 3;
        //printk("NT_TASKS+ NR_NATIVE_PROCS = %d\n",NR_TASKS + NR_NATIVE_PROCS);
        /*insert into running queue*/
        insert_rq(p_proc);
    }
    //proc_table[1].signal |= (1 << (2));

    k_reenter	= 0;
    ticks		= 0;

    current 	= proc_table;
}

int pretty_main()
{
    init_task();
    start_kernel();
    return 0;
}
