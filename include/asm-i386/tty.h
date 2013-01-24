#ifndef	_TTY_H_
#define	_TTY_H_

#include "sched.h"
#include "wait.h"

#define TTY_IN_BYTES	256	

#define MAG_CH_PANIC	'\006'
#define MAG_CH_ASSERT 	'\007'
#define RED_COLOR 0x4

typedef struct s_tty
{
	t32	in_buf[TTY_IN_BYTES];
	t32*	p_inbuf_head;		
	t32*	p_inbuf_tail;		
	int	inbuf_count;		

	struct s_console *	p_console;
}TTY;


extern TTY tty_table[];
extern TTY *TTY_FIRST;
extern TTY *TTY_END;

extern void	task_tty();
extern void	in_process(TTY* p_tty, t32 key);
extern int 	sys_printx(char *buf,int len);
extern int  init_tty();
extern void tty_do_read(TTY* p_tty);

#endif 
