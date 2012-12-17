
#ifndef	_CONST_H_
#define	_CONST_H_

#define NULL	0

/* Boolean */

#define	TRUE	1
#define	FALSE	0

/* Color */
/*
* e.g.	MAKE_COLOR(BLUE, RED)
*	MAKE_COLOR(BLACK, RED) | BRIGHT
*	MAKE_COLOR(BLACK, RED) | BRIGHT | FLASH
*/
#define	BLACK	0x0	/* 0000 */
#define	WHITE	0x7	/* 0111 */
#define	RED	0x4	/* 0100 */
#define	GREEN	0x2	/* 0010 */
#define	BLUE	0x1	/* 0001 */
#define	FLASH	0x80	/* 1000 0000 */
#define	BRIGHT	0x08	/* 0000 1000 */
#define	MAKE_COLOR(x,y)	((x<<4) | y)	/* MAKE_COLOR(Background,Foreground) */

/* GDT  IDT size */
#define	GDT_SIZE	128
#define	IDT_SIZE	256

/* privilege*/
#define	PRIVILEGE_KRNL	0
#define	PRIVILEGE_TASK	1
#define	PRIVILEGE_USER	3
/* RPL */
#define	RPL_KRNL	SA_RPL0
#define	RPL_TASK	SA_RPL1
#define	RPL_USER	SA_RPL3

/* TTY */
#define	NR_CONSOLES	3	/* consoles */

/* 8259A interrupt controller ports. */
#define	INT_M_CTL	0x20	/* I/O port for interrupt controller         <Master> */
#define	INT_M_CTLMASK	0x21	/* setting bits in this port disables ints   <Master> */
#define	INT_S_CTL	0xA0	/* I/O port for second interrupt controller  <Slave>  */
#define	INT_S_CTLMASK	0xA1	/* setting bits in this port disables ints   <Slave>  */

/* 8042 ports */
#define	KB_DATA		0x60	/* I/O port for keyboard data
					Read : Read Output Buffer 
					Write: Write Input Buffer(8042 Data&8048 Command) */
#define	KB_CMD		0x64	/* I/O port for keyboard command
					Read : Read Status Register
					Write: Write Input Buffer(8042 Command) */
#define	LED_CODE	0xED
#define	KB_ACK		0xFA

/* VGA */
#define CRTC_ADDR_REG			0x3D4	/* CRT Controller Registers - Address Register */
#define CRTC_DATA_REG			0x3D5	/* CRT Controller Registers - Data Registers */
#define CRTC_DATA_IDX_START_ADDR_H	0xC	/* register index of video mem start address (MSB) */
#define CRTC_DATA_IDX_START_ADDR_L	0xD	/* register index of video mem start address (LSB) */
#define CRTC_DATA_IDX_CURSOR_H		0xE	/* register index of cursor position (MSB) */
#define CRTC_DATA_IDX_CURSOR_L		0xF	/* register index of cursor position (LSB) */
#define V_MEM_BASE			0xB8000	/* base of color video memory */
#define V_MEM_SIZE			0x8000	/* 32K: B8000H -> BFFFFH */


/* Hardware interrupts */
#define	NR_IRQ		16	/* Number of IRQs */
#define	CLOCK_IRQ	0
#define	KEYBOARD_IRQ	1
#define	CASCADE_IRQ	2	/* cascade enable for 2nd AT controller */
#define	ETHER_IRQ	3	/* default ethernet interrupt vector */
#define	SECONDARY_IRQ	3	/* RS232 interrupt vector for port 2 */
#define	RS232_IRQ	4	/* RS232 interrupt vector for port 1 */
#define	XT_WINI_IRQ	5	/* xt winchester */
#define	FLOPPY_IRQ	6	/* floppy disk */
#define	PRINTER_IRQ	7
#define	HD_IRQ		14	/* at winchester */

/* system call */
#define	NR_SYS_CALL	6

#define ASSERT

#ifdef 	ASSERT
void assertion_failure(char *exp,char *file,char *base_file,int line);
#define assert(exp) do{ \
    if(exp);\
        else	 \
    assertion_failure(#exp,__FILE__,__BASE_FILE__,__LINE__); \
}while(0)
#else
#define assert(exp)
#endif

#define MAG_CH_PANIC	'\006'
#define MAG_CH_ASSERT 	'\007'

#define RED_COLOR 0x4
/* signal.c */
#define NR_SIGNALS	32

#define MIN(a,b)	(((a)>(b)?(b):(a)))
#define MAX(a,b)	(((a)>(b)?(a):(b)))

#define ALIGN(value,agn) ((value) & (~(agn) + 1))

#endif /* _Pretty_CONST_H_ */

