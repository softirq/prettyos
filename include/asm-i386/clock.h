#ifndef     _CLOCK_H_
#define     _CLOCK_H_

/* 8253/8254 PIT (Programmable Interval Timer) */
#define TIMER0          0x40	/* I/O port for timer channel 0 */
#define TIMER_MODE      0x43	/* I/O port for timer mode control */
#define RATE_GENERATOR	0x34	/* 00-11-010-0 : Counter0 - LSB then MSB - rate generator - binary */
#define TIMER_FREQ	1193182L/* clock frequency for timer in PC and AT */
#define HZ		100	/* clock freq (software settable on IBM-PC) */

extern  long 		jiffies;
extern	int		ticks;

extern void	clock_handler(int irq);
extern void	milli_delay(int milli_sec);
extern void 	init_clock();

#endif
