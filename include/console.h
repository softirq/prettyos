
#ifndef	_Pretty_CONSOLE_H_
#define	_Pretty_CONSOLE_H_


typedef struct s_console
{
	//struct s_tty*	p_tty;
	unsigned int	current_start_addr;	//当前屏幕的起始位置
	unsigned int	original_addr;		//console的开始的物理地址
	unsigned int    current_end_addr;	//当前屏幕的结束位置
	unsigned int	v_mem_limit;		//console的内存长度
	unsigned int	cursor;			//光标位置
}CONSOLE;


#define SCROLL_SCREEN_UP	1	/* scroll forward */
#define SCROLL_SCREEN_DOWN	-1	/* scroll backward */

#define SCREEN_WIDTH		80
#define SCREEN_HIGH		25
#define SCREEN_SIZE		(SCREEN_WIDTH * SCREEN_HIGH)

#define DEFAULT_CHAR_COLOR	0x07	


#endif 
