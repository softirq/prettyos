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
#include "kernel.h"
#include "lib.h"

static void reserved_irq(int irq);

void init_8259A()
{
    out_byte(INT_M_CTL,	0x11);			// Master 8259, ICW1.
    out_byte(INT_S_CTL,	0x11);			// Slave  8259, ICW1.
    out_byte(INT_M_CTLMASK,	INT_VECTOR_IRQ0);	// Master 8259, ICW2. 
    out_byte(INT_S_CTLMASK,	INT_VECTOR_IRQ8);	// Slave  8259, ICW2.
    out_byte(INT_M_CTLMASK,	0x4);			// Master 8259, ICW3.
    out_byte(INT_S_CTLMASK,	0x2);			// Slave  8259, ICW3.
    out_byte(INT_M_CTLMASK,	0x1);			// Master 8259, ICW4.
    out_byte(INT_S_CTLMASK,	0x1);			// Slave  8259, ICW4.

    out_byte(INT_M_CTLMASK,	0xFF);	// Master 8259, OCW1. 
    out_byte(INT_S_CTLMASK,	0xFF);	// Slave  8259, OCW1. 

    int i;
    // 	default hardware interrupt handler	
    for(i=0;i<NR_IRQ;i++)
    {
        //default irq handler
        irq_table[i] = reserved_irq;
    }
}

//set interrupt handler
void put_irq_handler(int irq, irq_handler_ptr handler)
{
    disable_irq(irq);
    irq_table[irq] = handler;
}

//default irq handler
static void reserved_irq(int irq)
{
    disp_str("spurious_irq: ");
    disp_int(irq);
    disp_str("\n");
}
