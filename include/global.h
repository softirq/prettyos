#ifndef     _GLOBAL_H_
#define     _GLOBAL_H_

#ifdef	GLOBAL_VARIABLES_HERE
#undef	extern
#define	extern
#endif

#ifndef _V_JIFFIES
extern  long 		jiffies;
#endif

extern	t32		k_reenter;

extern	t8		gdt_ptr[6];	// 0~15:Limit  16~47:Base
extern	DESCRIPTOR	gdt[GDT_SIZE];
extern	t8		idt_ptr[6];	// 0~15:Limit  16~47:Base
extern	GATE		idt[IDT_SIZE];

extern	TSS		tss;
//extern	PROCESS*	p_proc_ready;
extern 	struct 	task_struct*	current;

extern	int		nr_current_console;

extern PROCESS		proc_table[];
extern char		task_stack[];
extern	TASK		task_table[];
extern	TASK		user_proc_table[];

extern	int		ticks;
//video memory position
extern	int		disp_pos;

extern	TTY		tty_table[];
extern	CONSOLE		console_table[];

extern	irq_handler_ptr	irq_table[];
extern	syscall_ptr		sys_call_table[];

#endif
