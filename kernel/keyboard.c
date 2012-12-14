
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
#include "keymap.h"
#include "kernel.h"
#include "lib.h"

static	KB_INPUT	kb_in;
static	tbool		code_with_E0	= FALSE;
//static t_bool 		tab;
static	tbool		shift_l;		/* l shift state	*/
static	tbool		shift_r;		/* r shift state	*/
static	tbool		alt_l;			/* l alt state		*/
static	tbool		alt_r;			/* r left state		*/
static	tbool		ctrl_l;			/* l ctrl state		*/
static	tbool		ctrl_r;			/* l ctrl state		*/
static	tbool		caps_lock;		/* Caps Lock		*/
static	tbool		num_lock;		/* Num Lock		*/
static	tbool		scroll_lock;		/* Scroll Lock		*/
static	int		column		= 0;	/* keyrow[column] ???? keymap ??ĳһ??ֵ */

static t8	get_byte_from_kb_buf();
static void	set_leds();
static void	kb_wait();
static void	kb_ack();

void keyboard_handler(int irq)
{
	t8 scan_code = in_byte(KB_DATA);
//	disp_str("keyboard handler\n");

	if (kb_in.count < KB_IN_BYTES) {
		*(kb_in.p_head) = scan_code;
		kb_in.p_head++;
		if (kb_in.p_head == kb_in.buf + KB_IN_BYTES) {
			kb_in.p_head = kb_in.buf;
		}
		kb_in.count++;
	}
}


void init_keyboard()
{
	kb_in.count = 0;
	kb_in.p_head = kb_in.p_tail = kb_in.buf;
	caps_lock	= 0;
	num_lock	= 1;
	scroll_lock	= 0;
	set_leds();
//keyboard interrupt 
	put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
	enable_irq(KEYBOARD_IRQ);		
}


void keyboard_read(TTY* p_tty)
{
	t8	scan_code;
	tbool	make;
	t32	key = 0;
	t32*	keyrow;	

	if(kb_in.count > 0)
	{
		code_with_E0 = FALSE;
		scan_code = get_byte_from_kb_buf();

		if (scan_code == 0xE1) 
		{
			int i;
			t8 pausebreak_scan_code[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
			tbool is_pausebreak = TRUE;
			for(i=1;i<6;i++)
			{
				if (get_byte_from_kb_buf() != pausebreak_scan_code[i]) {
					is_pausebreak = FALSE;
					break;
				}
			}
			if (is_pausebreak) 
			{
				key = PAUSEBREAK;
			}
		}
		else if (scan_code == 0xE0) 
		{
			code_with_E0 = TRUE;
			scan_code = get_byte_from_kb_buf();

			if (scan_code == 0x2A) 
			{
				code_with_E0 = FALSE;
				if ((scan_code = get_byte_from_kb_buf()) == 0xE0) 
				{
					code_with_E0 = TRUE;
					if ((scan_code = get_byte_from_kb_buf()) == 0x37) 
					{
						key = PRINTSCREEN;
						make = TRUE;
					}
				}
			}
			else if (scan_code == 0xB7) 
			{
				code_with_E0 = FALSE;
				if ((scan_code = get_byte_from_kb_buf()) == 0xE0) 
				{
					code_with_E0 = TRUE;
					if ((scan_code = get_byte_from_kb_buf()) == 0xAA) 
					{
						key = PRINTSCREEN;
						make = FALSE;
					}
				}
			}
		}
//???? PAUSEBREAK ?? PRINTSCREEN????????
		if ((key != PAUSEBREAK) && (key != PRINTSCREEN)) 
		{
			make = (scan_code & FLAG_BREAK ? FALSE : TRUE);
			keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];
			column = 0;
			tbool caps = shift_l || shift_r;
			if (caps_lock) 
			{
				if ((keyrow[0] >= 'a') && (keyrow[0] <= 'z'))
				{
					caps = !caps;
				}
			}
			if (caps) 
			{
				column = 1;
			}

			if (code_with_E0) 
			{
				column = 2;
			}

//l_shift ????2a ?keyrow???? key = SHIFT_L; 
//?bool shift_l???true ??????? ?key ????????? ?????????????
			
			key = keyrow[column];

			switch(key) 
			{
//				case TAB:
//					tab = make;
//					break;
				case SHIFT_L:
					shift_l	= make;
					break;
				case SHIFT_R:
					shift_r	= make;
					break;
				case CTRL_L:
					ctrl_l	= make;
					break;
				case CTRL_R:
					ctrl_r	= make;
					break;
				case ALT_L:
					alt_l	= make;
					break;
				case ALT_R:
					alt_l	= make;
					break;
				case CAPS_LOCK:
					if (make) 
					{
						caps_lock   = !caps_lock;
						set_leds();
					}
					break;
				case NUM_LOCK:
					if (make) 
					{
						num_lock    = !num_lock;
						set_leds();
					}
					break;
				case SCROLL_LOCK:
					if (make) 
					{
						scroll_lock = !scroll_lock;
						set_leds();
					}
					break;
				default:
					break;
			}
		}

		if(make)
		{
			tbool pad = FALSE;
			if ((key >= PAD_SLASH) && (key <= PAD_9)) 
			{
				pad = TRUE;
				switch(key) 
				{	
					case PAD_SLASH:
						key = '/';
						break;
					case PAD_STAR:
						key = '*';
						break;
					case PAD_MINUS:
						key = '-';
						break;
					case PAD_PLUS:
						key = '+';
						break;
					case PAD_ENTER:
						key = ENTER;
						break;
					default:
						if (num_lock)
						{
							if ((key >= PAD_0) && (key <= PAD_9)) 
							{
								key = key - PAD_0 + '0';
							}
							else if (key == PAD_DOT) 
							{
								key = '.';
							}
						}
						else
						{
							switch(key) 
							{
								case PAD_HOME:
									key = HOME;
									break;
								case PAD_END:
									key = END;
									break;
								case PAD_PAGEUP:
									key = PAGEUP;
									break;
								case PAD_PAGEDOWN:
									key = PAGEDOWN;
									break;
								case PAD_INS:
									key = INSERT;
									break;
								case PAD_UP:
									key = UP;
									break;
								case PAD_DOWN:
									key = DOWN;
									break;
								case PAD_LEFT:
									key = LEFT;
									break;
								case PAD_RIGHT:
									key = RIGHT;
									break;
								case PAD_DOT:
									key = DELETE;
									break;
								default:
									break;
							}
						}
						break;
				}
			}
			//			key |= tab	? FLAG_TAB	: 0;
			key |= shift_l	? FLAG_SHIFT_L	: 0;
			key |= shift_r	? FLAG_SHIFT_R	: 0;
			key |= ctrl_l	? FLAG_CTRL_L	: 0;
			key |= ctrl_r	? FLAG_CTRL_R	: 0;
			key |= alt_l	? FLAG_ALT_L	: 0;
			key |= alt_r	? FLAG_ALT_R	: 0;
			key |= pad	? FLAG_PAD	: 0;

			in_process(p_tty, key);
		}
	}
}


static t8 get_byte_from_kb_buf()	
{
	t8	scan_code;

	while (kb_in.count <= 0) {}

	disable_int();
	scan_code = *(kb_in.p_tail);
	kb_in.p_tail++;
	if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES) 
	{
		kb_in.p_tail = kb_in.buf;
	}
	kb_in.count--;
	enable_int();

#ifdef __TINIX_DEBUG__
	disp_color_str("[", MAKE_COLOR(WHITE,BLUE));
	disp_int(scan_code);
	disp_color_str("]", MAKE_COLOR(WHITE,BLUE));
#endif

	return scan_code;
}


static void kb_wait()
{
	t8 kb_stat;

	do 
	{
		kb_stat = in_byte(KB_CMD);
	} while (kb_stat & 0x02);
}


static void kb_ack()
{
	t8 kb_read;

	do 
	{
		kb_read = in_byte(KB_DATA);
	} while (kb_read != KB_ACK);
}


static void set_leds()
{
	t8 leds = (caps_lock << 2) | (num_lock << 1) | scroll_lock;

	kb_wait();
	out_byte(KB_DATA, LED_CODE);
	kb_ack();

	kb_wait();
	out_byte(KB_DATA, leds);
	kb_ack();
}


