#ifndef     _TIMER_H_
#define     _TIMER_H_

struct timeval 
{
		unsigned long tv_sec; 		//second
		unsigned long tv_usec; 		//microsecond
};

struct timer_list 
{
	struct timer_list *prev;
	struct timer_list *next;
	unsigned long  	expires;
	unsigned long 	data;
	void (*function)(unsigned long);
};

extern int  del_timer(struct timer_list * timer);  
extern int  add_timer(struct timer_list * timer);  
extern int  init_timer(struct timer_list *timer);

#ifndef _V_JIFFIES
extern long jiffies;
#endif

void do_gettimeofday(struct timeval *tv);

#endif
