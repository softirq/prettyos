#ifndef     _SCHED_H_
#define     _SCHED_H_

#include "signal.h"
#include "fs.h"
#include "mm.h"
#include "rbtree.h"
#include "asm-i386/processor.h"

struct cfs_rq
{
    unsigned long nr_running;

    struct rb_root task_timeline;
    struct rb_node *rb_leftmost;

    unsigned long rq_weight;
};

struct rq
{
    unsigned long nr_running;
    struct cfs_rq cfs;  /* cfs schedule */
};

struct sched_entity
{
    struct rb_node run_node;
    u64 vruntime;
};

struct sched_class
{
    const struct sched_class *next;

    void (*enqueue_task) (struct rq *rq, struct task_struct *p, int wakeup,bool head);
    void (*dequeue_task) (struct rq *rq, struct task_struct *p, int sleep);
    struct task_struct * (*pick_next_task) (struct rq *rq);

    void (*switched_from) (struct rq *this_rq, struct task_struct *task,int running);
    void (*switched_to) (struct rq *this_rq, struct task_struct *task,int running);
    void (*prio_changed) (struct rq *this_rq, struct task_struct *task,int oldprio, int running);
};

typedef struct s_stackframe {	
    t32	gs;		
    t32	fs;		
    t32	es;		
    t32	ds;		
    t32	edi;		
    t32	esi;		
    t32	ebp;		
    t32	kernel_esp;	
    t32	ebx;		
    t32	edx;		
    t32	ecx;		
    t32	eax;		
    t32	retaddr;	
    t32	eip;		
    t32	cs;		
    t32	eflags;		
    t32	esp;		
    t32	ss;		
}STACK_FRAME;

typedef struct task_struct 
{
    STACK_FRAME	regs;			
    t16		ldt_sel;		
    DESCRIPTOR	ldts[LDT_SIZE];		
    struct 	sigaction sig_action[32];//32个信号 
    int 		state;
    unsigned long signal; //信号位图
    unsigned long blocked; //信号屏蔽字段
    tbool		sigpending;	//是否有信号 有true 没有false
    int 		flags;
    int		ticks;			
    int		priority;
    int 		nr_tty;
    //int 		parent;
    t32		pid;			
    char		name[16];		
    struct 	file 	*filp[NR_OPEN];
    //	struct task_struct *parent;
    struct m_inode 	*pwd;
    struct m_inode 	*root;
    struct m_inode 	*executable;
    struct thread_struct tss;
    struct mm_struct *mm;
    int 		exit_code;
    struct task_struct *parent, *next, *sibling;
    struct sched_class *sched_class;
    struct sched_entity se;

}PROCESS;

typedef struct s_task 
{
    t_pf_task	initial_eip;
    int			stacksize;
    char		name[32];
}TASK;

extern struct task_struct *current;
extern struct task_struct *run_queue;
extern struct task_struct *init;

#define NR_SYSTEM_PROCS 	1 //system process : tty  
#define NR_USER_PROCS 		5 //user process : testA testB testC testD init
//total procs in this system
#define NR_PROCS		32 // 

#define NR_PROCESS		NR_SYSTEM_PROCS+ NR_USER_PROCS

#define STACK_SIZE_DEFAULT	0x4000
#define STACK_SIZE_TTY		STACK_SIZE_DEFAULT
#define STACK_SIZE_TESTA	STACK_SIZE_DEFAULT
#define STACK_SIZE_TESTB	STACK_SIZE_DEFAULT
#define STACK_SIZE_TESTC	STACK_SIZE_DEFAULT
#define STACK_SIZE_TESTD	STACK_SIZE_DEFAULT
#define STACK_SIZE_INIT		STACK_SIZE_DEFAULT

#define STACK_SIZE_TOTAL	(STACK_SIZE_TTY + \
        STACK_SIZE_INIT + \
        STACK_SIZE_TESTA + \
        STACK_SIZE_TESTB + \
        STACK_SIZE_TESTC + \
        STACK_SIZE_TESTD)

#define proc2pid(p) (p - proc_table)
#define pid2proc(pid) (proc_table + pid)

/* 	null		0 */
#define FREE_SLOT	0
//#define NO_PARENT 	NULL

#define TASK_RUNNING		0	//running
#define TASK_INTERRUPTIBLE	1	//sleep 
#define TASK_UNINTERRUPTIBLE 	2	//wait
#define TASK_ZOMBIE		3	//being terminated
#define TASK_STOPPED		4	//being traced
#define TASK_WAITING 	TASK_UNINTERRUPTIBLE

int insert_rq(struct task_struct *p);
int delete_rq(struct task_struct *p);

#endif
