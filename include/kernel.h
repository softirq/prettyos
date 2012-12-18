#ifndef     _KERNEL_H_
#define     _KERNEL_H_

/* traps.c */
extern void	init_trap();
// t32	seg2phys(t_16 seg);
extern void	disable_irq(int irq);
extern void	enable_irq(int irq);
extern void init_descriptor(DESCRIPTOR * p_desc, t32 base, t32 limit, t16 attribute);

/* core.asm */
extern void	restart();
extern void	move_to_user_mode();

/* main.c */
extern void	TestA();
extern void	TestB();
extern void	TestC();
extern void	TestD();
extern void 	init_p();

/* i8259.c */
extern void	put_irq_handler(int iIRQ, irq_handler_ptr handler);
// void	spurious_irq(int irq);
extern void 	init_8259A();

/* clock.c */
extern void	clock_handler(int irq);
extern void	milli_delay(int milli_sec);
extern void 	init_clock();

/* sched.c */
extern void 	wake_up(struct  wait_queue **wq);
extern void 	sleep_on(struct wait_queue **wq);
extern void	schedule();
extern int 	goodness(PROCESS **p);
extern void 	switch_to(PROCESS *prev,PROCESS *next);

/* keyboard.c */
extern void	keyboard_handler(int irq);
extern void	keyboard_read(TTY* p_tty);
extern void 	init_keyboard();

/* tty.c */
extern void	task_tty();
extern void	in_process(TTY* p_tty, t32 key);
extern int 	sys_printx(char *buf,int len,struct task_struct *p_proc);

/* console.c */
extern void	init_screen(TTY* p_tty);
extern void	out_char(CONSOLE* p_con, char ch);
extern void	scroll_screen(CONSOLE* p_con, int direction);
tbool	is_current_console(CONSOLE* p_con);
extern void select_console(int nr_console);
/* syscall.asm 
   void	sys_call();		
   int	get_ticks();
   void 	write(char *buf,int len);
   void 	printx(char *buf,int len);
   */

/* sys.c */
int	sys_get_ticks();
// 	int 	sys_write(char *buf,int len,PROCESS *p_proc);

/*	proc.c 	*/
extern void* 	va2la(int pid,void *va);

/* signal.c */
extern void 	do_signal(int signr);
extern int 	get_signal_bitmap();
extern int 	send_signal(int signr,int pid);

/* hd.c */
extern void 	ha_handler(int irq);
extern void 	init_hd();
extern void 	hd_identify(int drive);
extern void 	hd_rw(int net_device,int start_sect,int nr_sects,int flag,struct buffer_head *bh);
extern void 	hd_open(int net_device);
// void do_signal();


#define 	suser() (!current->euid == 0)

#define container_of(ptr, type, memeber) ({ \
        const typeof((type *)0->member) *_mptr = ptr;\
        (type*)((char *)_mptr - offsetof(type,member));)}

#endif
