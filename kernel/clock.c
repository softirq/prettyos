#include "type.h"
#include "const.h"
#include "list.h"
#include "wait.h"
#include "traps.h"
#include "timer.h"
//#include "string.h"
/*#include "tty.h"*/
/*#include "console.h"*/
/*#include "mm.h"*/
#include "irq.h"
#include "sched.h"
/*#include "global.h"*/
/*#include "kernel.h"*/
#include "syscall.h"
#include "stdlib.h"
#include "clock.h"

long jiffies = 0;
int ticks = 0;

void clock_handler(int irq)
{
    ++jiffies;
    ++ticks;
    --current->ticks;
    /*vruntime increased and cfs choose the smallest one*/
    ++current->sched_entity.vruntime;

    //interrupt reenter
    if (k_reenter != 0) 
    {
        return;
    }

    //add something about timer
    //
    if(jiffies%NR_PROCESS == 0)
        schedule();
}

//every clock interrupt is 10ms HZ=100
void milli_delay(int milli_sec)
{
    int t = get_ticks();
    while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

/*init timer*/
void init_clock()
{
    out_byte(TIMER_MODE, RATE_GENERATOR);

    //set HZ = 100
    out_byte(TIMER0, (t8) (TIMER_FREQ/HZ) );
    out_byte(TIMER0, (t8) ((TIMER_FREQ/HZ) >> 8));

    put_irq_handler(CLOCK_IRQ, clock_handler);	
    enable_irq(CLOCK_IRQ);			
}
