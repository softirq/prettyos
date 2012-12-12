#include "type.h"
#include "const.h"
#include "traps.h"
//#include "string.h"
#include "tty.h"
#include "console.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "global.h"
#include "keyboard.h"
#include "kernel.h"
#include "lib.h"


static void	set_cursor(unsigned int position);
static void	set_video_start_addr(t_32 addr);
static void	flush(CONSOLE* p_con);
static int 	clean_console_screen(CONSOLE *p_con);

//输入换行符号之前 统计输入的字符个数
static int out_char_number = 0;

void out_str(CONSOLE *p_con,char *str,int len)
{
	int i = 0;
	t_8 *p_vmem = (t_8*)(V_MEM_BASE +  p_con->cursor * 2);
	for(;i < len;i++)
	{
		if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 1) 
		{
			*p_vmem++ = *str;
			*p_vmem++ = DEFAULT_CHAR_COLOR;
			str++;
			p_con->cursor++;
		}
	}
	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) 
	{
		scroll_screen(p_con, SCROLL_SCREEN_DOWN);
	}
	flush(p_con);

}

void out_char(CONSOLE* p_con, char ch)
{
	t_8* p_vmem = (t_8*)(V_MEM_BASE + p_con->cursor * 2);

	switch(ch) {
		case '\n':
			if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - SCREEN_WIDTH) 
			{
				p_con->cursor = p_con->original_addr + SCREEN_WIDTH * ((p_con->cursor - p_con->original_addr) / SCREEN_WIDTH + 1);
//				out_str(p_con,"[SunKang@localhost bash]>",25);
			}
			else
			{

				scroll_screen(p_con, SCROLL_SCREEN_DOWN);
				p_con->cursor = p_con->original_addr;
//				out_str(p_con,"[SunKang@localhost bash]>",25);
			}
			out_char_number = 0;
			break;
		case '\b':
			if (p_con->cursor > p_con->original_addr) 
			{
				//				disp_int(out_char_number);
				//			if((p_con->cursor%80) > 25)
				if(out_char_number > 0)
				{
					p_con->cursor--;
					*(p_vmem-2) = ' ';
					*(p_vmem-1) = DEFAULT_CHAR_COLOR;
					out_char_number--;
				}
			}
			break;
		case '\t':
			out_char_number += 8;
			p_con->cursor += 8 ;
			//		p_vmem += 8 * 2;
			break;	

		default:
			if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 1) 
			{
				*p_vmem++ = ch;
				*p_vmem++ = DEFAULT_CHAR_COLOR;
				p_con->cursor++;
				out_char_number++;
			}
			break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) 
	{
		scroll_screen(p_con, SCROLL_SCREEN_DOWN);
	}

	flush(p_con);
}


t_bool is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


static void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}


static void set_video_start_addr(t_32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}


void select_console(int nr_console)
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) 
	{
		return;
	}

	nr_current_console = nr_console;

	flush(&console_table[nr_console]);
}


void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCROLL_SCREEN_UP) 
	{
		if (p_con->current_start_addr > p_con->original_addr) 
		{
			p_con->current_start_addr -= SCREEN_WIDTH;
			p_con->current_end_addr -= SCREEN_WIDTH;
			p_con->cursor -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCROLL_SCREEN_DOWN) 
	{
		if (p_con->current_start_addr + SCREEN_SIZE < p_con->original_addr + p_con->v_mem_limit) 
		{
			p_con->current_start_addr += SCREEN_WIDTH;
			//	p_con->cursor += SCREEN_WIDTH;
			if(p_con->current_end_addr + SCREEN_WIDTH > p_con->original_addr + p_con->v_mem_limit)
				p_con->current_end_addr = p_con->original_addr +  p_con->v_mem_limit;
			else 
				p_con->current_end_addr += SCREEN_WIDTH;
		}
		else //
		{
			disp_pos = 0;	
			clean_console_screen(p_con);
			p_con->current_start_addr = p_con->original_addr;
			p_con->current_end_addr += p_con->current_start_addr + SCREEN_SIZE;
//			out_str(p_con,"[SunKang@localhost bash]>",25);
//			p_con->cursor = p_con->current_start_addr;
		}

	}
	else
	{
		//		out_str(p_con,"no memory",10);
	}

	flush(p_con);
}


static void flush(CONSOLE* p_con)
{
	set_cursor(p_con->cursor);
	set_video_start_addr(p_con->current_start_addr);
}

void init_screen(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;

	int con_v_mem_size			= v_mem_size / NR_CONSOLES;		

	p_tty->p_console->original_addr		= nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit		= con_v_mem_size / SCREEN_WIDTH * SCREEN_WIDTH;	
	p_tty->p_console->current_start_addr	= p_tty->p_console->original_addr;
	p_tty->p_console->current_end_addr 	= p_tty->p_console->current_start_addr + SCREEN_SIZE;
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) 
	{
		//		p_tty->p_console->cursor = disp_pos / 2;
		//delay a mement for looking messages
		p_tty->p_console->cursor =  SCREEN_SIZE;
		//		p_tty->p_console->cursor = 0;
		set_cursor(p_tty->p_console->cursor);
		milli_delay(5000);
		int i = 0;
		while(i < SCREEN_HIGH)	
		{
			scroll_screen(p_tty->p_console, SCROLL_SCREEN_DOWN);
			i++;
		}

		disp_pos = SCREEN_SIZE * 2;
		p_tty->p_console->cursor =  SCREEN_SIZE;
	//	out_str(p_tty->p_console,"[SunKang@localhost bash]>",25);
	}
	else 
	{
		//		out_char(p_tty->p_console, nr_tty + '0');
//		out_str(p_tty->p_console,"[SunKang@localhost bash]>",25);
		//		out_char(p_tty->p_console, '#');
	}
	set_cursor(p_tty->p_console->cursor);
}

static int clean_console_screen(CONSOLE *p_con)
{
	unsigned int len = p_con->v_mem_limit;
	unsigned int original_addr = p_con->original_addr << 1;

	__asm__ (
			"mov %%gs,%%ax\n\t"
			"mov %%ax,%%es\n\t" 
			"cld\n\t" 
			"rep\n\t"
			"stosw"
			:
			:"D"((long)original_addr),"a"((long)0x720),"c"((long)len)
		);
	return 0;
}
