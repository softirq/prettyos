#ifndef	_TTY_H_
#define	_TTY_H_

#define TTY_IN_BYTES	256	

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

#endif 
