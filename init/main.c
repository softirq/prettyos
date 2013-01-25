#include "type.h"
#include "const.h"
#include "traps.h"
#include "string.h"
#include "tty.h"
#include "console.h"
/*#include "wait.h"*/
#include "elf.h"
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
#include "clock.h"
#include "hd.h"

/*#define EXT_MEM_K 	(*(unsigned short*)0x8002)*/
//long memory_start = 0;
/*unsigned long buffer_memory_start = 0;*/
/*unsigned long buffer_memory_end = 0;*/
unsigned long main_memory_end = 0;
unsigned long main_memory_start = 0;
unsigned long memory_size = 0;

unsigned short selector_ldt	= SELECTOR_LDT_FIRST;
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

/* get kernel elf map */
static int get_kernel_map(unsigned int *base, unsigned int *limit)
{
    struct boot_params bp;
    get_boot_params(&bp);
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)(bp.kernel_addr);

    return get_elf_map(bp.kernel_addr, elf_header, base, limit);
}

/*init the first process*/
static void init_kernel_thread()
{
    TASK* p_task = NULL;
    /*struct task_struct* p_proc	= proc_table;*/
    struct task_struct *tsk = NULL;
    union thread_union *thread_union = NULL;
    char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
    int i,j,prio,eflags;
    char privilege,rpl;

    unsigned int k_base,k_limit;
    get_kernel_map(&k_base,&k_limit);

    privilege = PRIVILEGE_TASK;
    rpl = RPL_TASK;
    eflags = 0x1202;
    prio = KERNEL_PRIOR;

    printk("  PrettyOS SoftIRQ.\n");
    printk("  kangs.uestc@gmail.com .\n");
    printk("  UESTC.\n\n\n\n");

    /*disp_str("\t\tprocess init begins\n");*/
    /*for(i = 0;i < NR_SYSTEM_PROCS ;++i, ++p_proc)*/
    for(i = 0;i < NR_SYSTEM_PROCS ;++i)
    {
        p_task = task_table + i;

        tsk = (struct task_struct *)kmem_get_obj(tsk_cachep);
        if(tsk == NULL)
            return;

        thread_union = (union thread_union*)kmem_get_obj(thread_union_cachep);
        if(thread_union == NULL)
            return;

        thread_union->thread_info.task = tsk;

        tsk->state = TASK_RUNNING;
        strcpy(tsk->command, p_task->command);	// name of the process
        if((tsk->pid = get_pidmap()) < 0)
            return;
        tsk->parent = NULL;
        tsk->next = tsk->sibling = NULL;

        /*proc_table[0].nr_tty = 0;		// tty */
        for(j = 0; j < NR_SIGNALS;++j)
        {
            tsk->sig_action[j].sa_flags = 0;
            tsk->sig_action[j].sa_handler = do_signal;
        }
        for(j = 0;j < NR_OPEN; ++j)
        {
            tsk->filp[j] = NULL;
        }
        tsk->signal = 0x0; //设置信号为空
        tsk->ldt_sel = selector_ldt;

        init_descriptor(&gdt[selector_ldt>>3],vir2phys(seg2phys(SELECTOR_KERNEL_DS), tsk->ldts),LDT_SIZE * sizeof(DESCRIPTOR) - 1,DA_LDT);

        /*init_descriptor(&p_proc->ldts[INDEX_LDT_C],0,(k_base + k_limit) >> LIMIT_4K_SHIFT,DA_32 | DA_LIMIT_4K | DA_C| privilege <<5);*/
        /*init_descriptor(&p_proc->ldts[INDEX_LDT_D],0,(k_base + k_limit) >> LIMIT_4K_SHIFT,DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5);*/

        init_descriptor(&tsk->ldts[INDEX_LDT_C],0,(k_base + k_limit) >> LIMIT_4K_SHIFT,DA_32 | DA_LIMIT_4K | DA_C| privilege <<5);
        init_descriptor(&tsk->ldts[INDEX_LDT_D],0,(k_base + k_limit) >> LIMIT_4K_SHIFT,DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5);
        tsk->regs.esp	= (unsigned int)p_task_stack;
        p_task_stack -= STACK_SIZE_DEFAULT;

        tsk->regs.cs = (unsigned int)(((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.ds = (unsigned int)(((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.es = (unsigned int)(((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.fs = (unsigned int)(((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.ss = (unsigned int)(((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.gs = (unsigned int)((SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl);
        tsk->regs.eip = (unsigned int)p_task->initial_eip;

        tsk->regs.eflags = eflags;	
        tsk->ticks = tsk->priority = prio;
        tsk->sched_entity.vruntime = i;
        tsk->sched_class = &rr_sched;
        if(tsk->pid != INIT_PID)
            tsk->sched_class->enqueue_task(&(sched_rq),tsk,0,0);

        p_task++;
        selector_ldt += 1 << 3;
    }

    k_reenter	= 0;
    ticks		= 0;
}

/*init the first process*/
static void init_user_process()
{
    TASK* p_task = NULL;
    /*struct task_struct* p_proc	= proc_table + NR_SYSTEM_PROCS;*/
    struct task_struct *tsk = NULL;
    union thread_union *thread_union = NULL;

    int i,j,prio, eflags;
    char privilege,rpl;

    unsigned int k_base,k_limit;
    get_kernel_map(&k_base,&k_limit);

    privilege = PRIVILEGE_USER;
    rpl = RPL_USER;
    eflags = 0x1202;
    prio = USER_PRIO;

    /*for(i = 0;i < NR_USER_PROCS; ++i,++p_proc)*/
    for(i = 0;i < NR_USER_PROCS; ++i)
    {
        p_task = user_proc_table + i ;

        tsk = (struct task_struct *)kmem_get_obj(tsk_cachep);
        if(tsk == NULL)
            return;

        thread_union = (union thread_union*)kmem_get_obj(thread_union_cachep);
        if(thread_union == NULL)
            return;

        thread_union->thread_info.task = tsk;

        tsk->state = TASK_RUNNING;
        strcpy(tsk->command, p_task->command);	// name of the process
        if((tsk->pid = get_pidmap()) < 0)
            return;
        tsk->parent = NULL;
        tsk->next = tsk->sibling = NULL;

        for(j = 0; j < NR_SIGNALS;++j)
        {
            tsk->sig_action[j].sa_flags = 0;
            tsk->sig_action[j].sa_handler = do_signal;
        }
        for(j = 0;j < NR_OPEN; ++j)
        {
            tsk->filp[j] = NULL;
        }
        tsk->signal = 0x0; //设置信号为空
        tsk->ldt_sel = selector_ldt;

        init_descriptor(&gdt[selector_ldt>>3],vir2phys(seg2phys(SELECTOR_KERNEL_DS), tsk->ldts),LDT_SIZE * sizeof(DESCRIPTOR) - 1,DA_LDT);
        /*init_descriptor(&gdt[selector_ldt>>3],vir2phys(seg2phys(SELECTOR_KERNEL_DS), p_proc->ldts),LDT_SIZE * sizeof(DESCRIPTOR) - 1,DA_LDT);*/

        /*p_proc->ldts[INDEX_LDT_C] = gdt[SELECTOR_KERNEL_CS >> 3];*/
        tsk->ldts[INDEX_LDT_C] = gdt[SELECTOR_KERNEL_CS >> 3];
        /*p_proc->ldts[INDEX_LDT_C].attr1 = DA_C | privilege << 5;// change the DPL*/
        tsk->ldts[INDEX_LDT_C].attr1 = DA_C | privilege << 5;// change the DPL
        /*p_proc->ldts[INDEX_LDT_D] = gdt[SELECTOR_KERNEL_DS >> 3];*/
        tsk->ldts[INDEX_LDT_D] = gdt[SELECTOR_KERNEL_DS >> 3];
        /*p_proc->ldts[INDEX_LDT_D].attr1 = DA_DRW | privilege<< 5;// change the DPL*/
        tsk->ldts[INDEX_LDT_D].attr1 = DA_DRW | privilege<< 5;// change the DPL

        char *stack = (char *)thread_union->stack;
        tsk->regs.esp = (unsigned int)(stack + sizeof(union thread_union));

        tsk->regs.cs = (unsigned int)(((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.ds = (unsigned int)(((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.es = (unsigned int)(((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.fs = (unsigned int)(((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.ss = (unsigned int)(((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl);
        tsk->regs.gs = (unsigned int)((SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl);
        tsk->regs.eip = (unsigned int)p_task->initial_eip;

        tsk->regs.eflags = eflags;	
        tsk->ticks = tsk->priority = prio;

        tsk->sched_entity.vruntime = i;
        tsk->sched_class = &rr_sched;
        tsk->sched_class->enqueue_task(&(sched_rq),tsk,0,0);

        p_task++;
        selector_ldt += 1 << 3;
    }

    k_reenter	= 0;
    ticks		= 0;
}

static void init_task()
{
    init_kernel_thread();
    init_user_process();
}

/* choose a task and begin to run */
static void run_task()
{
    struct rb_root *root = &(cfs_runqueue.task_timeline);
    struct sched_entity *se = ts_leftmost(root);
    current = se_entry(se, struct task_struct, sched_entity);

    move_to_user_mode();
}

static void start_kernel()
{
    init_clock(); //clock interrupt init

    get_memsize(&main_memory_end);
    main_memory_end &= 0xfffff000;

    paging_init();
    init_mem(); //memeory management init
    buffer_init();
    /* scheduler init */
    init_sched(); 

    init_hd(); //hard disk init

    init_fs(); //filesystem init

    init_task();

    /*init_sock();*/

}

/* the kernel main func */
int pretty_main()
{
    start_kernel();

    run_task();

    /* from kernel mode to user mode and scheduler process */
    return 0;
}
