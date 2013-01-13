#ifndef	_TTY_H_
#define	_TTY_H_

#include "sched.h"
#include "wait.h"

#define TTY_IN_BYTES	256	

#define MAG_CH_PANIC	'\006'
#define MAG_CH_ASSERT 	'\007'
#define RED_COLOR 0x4

struct s_tty;
struct s_console;

typedef struct s_tty
{
	t32	in_buf[TTY_IN_BYTES];
	t32*	p_inbuf_head;		
	t32*	p_inbuf_tail;		
	int	inbuf_count;		

	struct s_console *	p_console;
}TTY;

extern	TTY		tty_table[];

extern void	task_tty();
extern void	in_process(TTY* p_tty, t32 key);
extern int 	sys_printx(char *buf,int len);

#endif 
