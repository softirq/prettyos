#ifndef     _PROC_H_
#define     _PROC_H_

#include "sched.h"

#define PROCS_BASE 		0xA00000
#define PROC_IMAGE_SIZE_DEFAULT 0x80000
#define PROC_ORIGIN_STACK 	0x400

//extern struct task_struct proc_table[];
extern char		    task_stack[];
extern	TASK		task_table[];
extern	TASK		user_proc_table[];

extern	TSS		tss;
extern 	struct 	task_struct*	current;

extern int get_limit(struct descriptor *dp);
extern int get_base(struct descriptor *dp);
//extern void* 	va2la(int pid,void *va);

#endif
