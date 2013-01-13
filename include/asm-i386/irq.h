#ifndef     _IRQ_H_
#define     _IRQ_H_

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

typedef void    (*irq_handler)  (int irq);
extern	irq_handler	    irq_table[];
extern void	put_irq_handler(int iIRQ, irq_handler handler);

extern void	disable_irq(int irq);
extern void	enable_irq(int irq);
extern void     disable_int();
extern void     enable_int();

#endif
