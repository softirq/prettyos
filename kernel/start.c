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
#include "string_32.h"

void cstart()
{
    //	disp_str("\n\n\n\n\n\n\nWelcome to Pretty OS\n");
    //	disp_str("\npretty start begins\n");

    memcpy(	&gdt,				    // New GDT
            (void*)(*((t32*)(&gdt_ptr[2]))),   // Base  of Old GDT
            *((t16*)(&gdt_ptr[0])) + 1	    // Limit of Old GDT
          );

    t16* p_gdt_limit = (t16*)(&gdt_ptr[0]);
    t32* p_gdt_base  = (t32*)(&gdt_ptr[2]);
    *p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
    *p_gdt_base  = (t32)&gdt;

    t16* p_idt_limit = (t16*)(&idt_ptr[0]);
    t32* p_idt_base  = (t32*)(&idt_ptr[2]);
    *p_idt_limit = IDT_SIZE * sizeof(GATE) - 1;
    *p_idt_base  = (t32)&idt;

    init_trap();

    //	disp_str("pretty start finished\n");
}
