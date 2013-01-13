#ifndef	_CONSOLE_H_
#define	_CONSOLE_H_

#include "tty.h"

typedef struct s_console
{
	//struct s_tty*	p_tty;
	unsigned int	current_start_addr;	//当前屏幕的起始位置
	unsigned int	original_addr;		//console的开始的物理地址
	unsigned int    current_end_addr;	//当前屏幕的结束位置
	unsigned int	v_mem_limit;		//console的内存长度
	unsigned int	cursor;			//光标位置
}CONSOLE;

extern	CONSOLE		console_table[];
extern	int		nr_current_console;
//video memory position
extern	int		disp_pos;

extern void	init_screen(TTY* p_tty);
extern void	out_char(CONSOLE* p_con, char ch);
extern void	scroll_screen(CONSOLE* p_con, int direction);
extern bool is_current_console(CONSOLE* p_con);
extern void select_console(int nr_console);

#define SCROLL_SCREEN_UP	1	/* scroll forward */
#define SCROLL_SCREEN_DOWN	-1	/* scroll backward */

#define SCREEN_WIDTH		80
#define SCREEN_HIGH		25
#define SCREEN_SIZE		(SCREEN_WIDTH * SCREEN_HIGH)

#define DEFAULT_CHAR_COLOR	0x07	
#define	NR_CONSOLES	3	/* consoles */

/* VGA */
#define CRTC_ADDR_REG			0x3D4	/* CRT Controller Registers - Address Register */
#define CRTC_DATA_REG			0x3D5	/* CRT Controller Registers - Data Registers */
#define CRTC_DATA_IDX_START_ADDR_H	0xC	/* register index of video mem start address (MSB) */
#define CRTC_DATA_IDX_START_ADDR_L	0xD	/* register index of video mem start address (LSB) */
#define CRTC_DATA_IDX_CURSOR_H		0xE	/* register index of cursor position (MSB) */
#define CRTC_DATA_IDX_CURSOR_L		0xF	/* register index of cursor position (LSB) */
#define V_MEM_BASE			0xB8000	/* base of color video memory */
#define V_MEM_SIZE			0x8000	/* 32K: B8000H -> BFFFFH */

#endif 
