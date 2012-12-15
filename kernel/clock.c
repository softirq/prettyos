#include "type.h"
#include "const.h"
#include "timer.h"
#include "traps.h"
//#include "string.h"
#include "tty.h"
#include "console.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "global.h"
#include "kernel.h"
#include "syscall.h"
#include "stdlib.h"

void clock_handler(int irq)
{
    jiffies++;
    ticks++;
    current->ticks--;

    //interrupt reenter
    if (k_reenter != 0) 
    {
        return;
    }

    //add something about timer
    //
    if (current->ticks > 0) 
    {
        return;
    }
    else 
    {
        schedule();
    }
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


