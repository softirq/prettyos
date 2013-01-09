#ifndef     _SCHED_H_
#define     _SCHED_H_

#include "signal.h"
#include "fs.h"
#include "mm.h"
#include "rbtree.h"
#include "list.h"
#include "fork.h"
#include "asm-i386/processor.h"
#include "asm-i386/traps.h"

#define     SCHED_RR    0x01
#define     SCHED_FIFO  0x02
#define     SCHED_RT    0x04
#define     SCHED_OTHER   0x08

#define     MIN_VRUNTIME    100

struct cfs_rq
{
    unsigned long cfs_nr_running;

    struct rb_root task_timeline;
    struct rb_node *rb_leftmost;

    unsigned long rq_weight;
    unsigned int min_vruntime;
    struct rq *rq;  /* back pointer to rq */
};

struct rt_rq
{
    unsigned long rt_nr_running;
    struct list_head rt_rq_list;
    struct rq *rq;  /* back pointer to rq */
};

struct fifo_rq
{
    unsigned long fifo_nr_running;
    struct list_head fifo_rq_list;
    struct rq *rq;  /* back pointer to rq */
};

struct rr_rq
{
    unsigned long rr_nr_running;
    struct list_head rr_rq_list;
    struct rq *rq;  /* back pointer to rq */
};

struct rq
{
    union {
        struct cfs_rq cfs;  /* cfs schedule */
        struct rt_rq rt;
        struct rr_rq rr;
        struct fifo_rq fifo;
    }u;
};

#define     rr_runqueue  (sched_rq.u.rr)
#define     rt_runqueue  (sched_rq.u.rt)
#define     cfs_runqueue  (sched_rq.u.cfs)
#define     fifo_runqueue  (sched_rq.u.fifo)

#define     CFS_RUNQUEUE(rq)    (rq->u.cfs)

#define se_entry(ptr, type, member) container_of(ptr, type, member)

struct sched_entity
{
    struct rb_node run_node;
    unsigned int vruntime;
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

typedef struct stackframe {	
    unsigned int    gs;		
    unsigned int    fs;		
    unsigned int    es;		
    unsigned int    ds;		
    unsigned int	edi;		
    unsigned int	esi;		
    unsigned int	ebp;		
    unsigned int	kernel_esp;	
    unsigned int	ebx;		
    unsigned int	edx;		
    unsigned int	ecx;		
    unsigned int	eax;		
    unsigned int	retaddr;	
    unsigned int	eip;		
    unsigned int    cs;		
    unsigned int	eflags;		
    unsigned int	esp;		
    unsigned int    ss;		
}STACK_FRAME;

typedef struct task_struct 
{
    struct stackframe regs;			
    t16		ldt_sel;		
    DESCRIPTOR	ldts[LDT_SIZE];		
    //struct 	sigaction sig_action[NR_SIGNALS];//32个信号 
    struct 	sigaction sig_action[NR_SIGNALS];//32个信号 
    int 		state;
    unsigned long signal; //信号位图
    unsigned long blocked; //信号屏蔽字段
    tbool		sigpending;	//是否有信号 有true 没有false
    int 		flags;
    int		ticks;			
    int		priority;
    int 		nr_tty;
    //int 		parent;
    pid_t pid;			
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
    struct sched_entity sched_entity;
    struct list_head list;
}PROCESS;

typedef struct s_task 
{
    t_pf_task	initial_eip;
    int			stacksize;
    char		name[32];
}TASK;

extern struct task_struct *current;
extern struct task_struct *init;

extern struct rq sched_rq;
extern struct sched_class rr_sched;

#define NR_SYSTEM_PROCS 	1 //system process : tty  
#define NR_USER_PROCS 		5 //user process : testA testB testC testD init
//total procs in this system
#define NR_PROCS		32 // 

#define NR_PROCESS		(NR_SYSTEM_PROCS + NR_USER_PROCS)

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

void init_sched();

#endif
