#include "type.h"
#include "const.h"
#include "traps.h"
#include "list.h"
#include "wait.h"
#include "tty.h"
#include "console.h"
#include "keyboard.h"
#include "irq.h"
/*#include "stdlib.h"*/

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

TTY			tty_table[NR_CONSOLES];

static void put_key(TTY* p_tty, t32 key)
{
    if (p_tty->inbuf_count < TTY_IN_BYTES) 
    {
        *(p_tty->p_inbuf_head) = key;
        p_tty->p_inbuf_head++;
        if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) 
        {
            p_tty->p_inbuf_head = p_tty->in_buf;
        }
        p_tty->inbuf_count++;
    }
}

static void tty_do_read(TTY* p_tty)
{
    if (is_current_console(p_tty->p_console)) 
    {
        keyboard_read(p_tty);
    }
}

static void tty_do_write(TTY* p_tty)
{
    if (p_tty->inbuf_count) 
    {
        char ch = *(p_tty->p_inbuf_tail);
        p_tty->p_inbuf_tail++;
        if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) 
        {
            p_tty->p_inbuf_tail = p_tty->in_buf;
        }
        p_tty->inbuf_count--;
        out_char(p_tty->p_console, ch);
    }
}

void task_tty()
{
    TTY *ptty = NULL;

    init_tty();
    select_console(0);

    while (1) 
    {
        for (ptty = TTY_FIRST; ptty < TTY_END; ++ptty) 
        {
            tty_do_read(ptty);
            tty_do_write(ptty);
        }
    }
}

int init_tty()
{
    TTY* p_tty = NULL;

    init_keyboard();

    for (p_tty = TTY_FIRST;p_tty < TTY_END;++p_tty) 
    {
        p_tty->inbuf_count = 0;
        p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;
        init_screen(p_tty);
    }

    return 0;
}

void in_process(TTY* p_tty, t32 key)
{
    if (!(key & FLAG_EXT)) 
    {
        put_key(p_tty, key);
    }
    else 
    {
        int raw_code = key & MASK_RAW;
        switch(raw_code) 
        {
            case ENTER:
                put_key(p_tty, '\n');
                break;
            case BACKSPACE:
                put_key(p_tty, '\b');
                break;
            case TAB:
                put_key(p_tty,'\t');
                break;
            case UP:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) 
                {	/* Shift + Up */
                    scroll_screen(p_tty->p_console, SCROLL_SCREEN_UP);
                }
                break;
            case DOWN:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) 
                {	/* Shift + Down */
                    scroll_screen(p_tty->p_console, SCROLL_SCREEN_DOWN);
                }
                break;
            case F1:
            case F2:
            case F3:
            case F4:
            case F5:
            case F6:
            case F7:
            case F8:
            case F9:
            case F10:
            case F11:
            case F12:
                if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) 
                {	/* Alt + F1~F12 */
                    select_console(raw_code - F1);
                }
                break;
            default:
                break;
        }
    }
}

void tty_write(TTY *p_tty,char *buf,int len)
{
    char *p = buf;
    int i = len;
    while(i)
    {
        out_char(p_tty->p_console,*p++);
        i--;
    }
}

/*int sys_printx(char *s,int len,struct task_struct *p_proc)*/
int sys_printx(char *s,int len)
{
    const char *p;
    char ch;
    char reenter_err[] = "? k_reenter is incorrect for unknown reason\n";

    if(k_reenter == 0)	//用户态下
    {
        /*p = (char *)va2la(proc2pid(p_proc),s);*/
    }
    else if(k_reenter > 0) //内核态
    {
        p =s;		
    }
    else
    {
        p = reenter_err;
    }

    ch = *p;
    /*if((ch == MAG_CH_PANIC) || (ch == MAG_CH_ASSERT && current < &proc_table[NR_SYSTEM_PROCS]))*/
    if((ch == MAG_CH_PANIC) || (ch == MAG_CH_ASSERT ))
    {
        disable_int(); //关中断
        char *v = (char *)V_MEM_BASE;
        const char *c = p + 1; //skip magic char
        while( v < ((char *)(V_MEM_BASE + V_MEM_SIZE)) && *c)
        {
            *v++ = *c++;
            *v++ = RED_COLOR; 
        }

        //halt processor until an enabled interrupt
        __asm__ __volatile__ ("hlt");
    }
    else
    {

        while((ch = *p++) != 0)	
        {
            out_char(tty_table[current->nr_tty].p_console,ch);
        }
    }

    return 0;
}

