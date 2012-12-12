
#include "type.h"
#include "const.h"
#include "traps.h"
#include "tty.h"
#include "console.h"
#include "wait.h"
#include "mm.h"
#include "sched.h"
#include "global.h"
#include "kernel.h"
//#include "string.h"
#include "syscall.h"
#include "lib.h"
#include "system.h"


static void init_idt_desc(unsigned int n, t_8 desc_type, t_pf_int_handler handler, unsigned char privilege);
static t_32 seg2phys(t_16 seg);

//static void init_descriptor(DESCRIPTOR * p_desc, t_32 base, t_32 limit, t_16 attribute);

//  this is the exception handler
extern void	divide_error();
extern void	single_step_exception();
extern void	nmi();
extern void	breakpoint_exception();
extern void	overflow();
extern void	bounds_check();
extern void	inval_opcode();
extern void	copr_not_available();
extern void	double_fault();
extern void	copr_seg_overrun();
extern void	inval_tss();
extern void	segment_not_present();
extern void	stack_exception();
extern void	general_protection();
extern void	page_fault();
extern void	copr_error();

// this is the hard interrupt IRQ0-IRQ15 handler

extern void	clock_intr(); //irq 0: clock interrupt 
extern void	kb_intr();    //irq 1: keyborad interrupt
extern void	hwint02();
extern void	hwint03();
extern void	hwint04();
extern void	hwint05();
extern void	hwint06();
extern void	hwint07();
extern void	hwint08();
extern void	hwint09();
extern void	hwint10();
extern void	hwint11();
extern void	hwint12();
extern void	hwint13();
extern void	hd_intr();   //irq 14: hard disk interrupt
extern void	hwint15();


void init_trap()
{
    int i;
    init_8259A();

    //soft interrupt
    set_trap_gate(INT_VECTOR_DIVIDE,divide_error);
    set_trap_gate(INT_VECTOR_DEBUG,		single_step_exception);
    set_trap_gate(INT_VECTOR_NMI,		nmi);
    set_system_gate(INT_VECTOR_BREAKPOINT,	breakpoint_exception);
    set_system_gate(INT_VECTOR_OVERFLOW, overflow);
    set_system_gate(INT_VECTOR_BOUNDS, bounds_check);
    set_trap_gate(INT_VECTOR_INVAL_OP, inval_opcode);
    set_trap_gate(INT_VECTOR_COPROC_NOT,	copr_not_available);
    set_trap_gate(INT_VECTOR_DOUBLE_FAULT, double_fault);
    set_trap_gate(INT_VECTOR_COPROC_SEG, copr_seg_overrun);
    set_trap_gate(INT_VECTOR_INVAL_TSS, inval_tss);
    set_trap_gate(INT_VECTOR_SEG_NOT, segment_not_present);
    set_trap_gate(INT_VECTOR_STACK_FAULT, stack_exception);
    set_trap_gate(INT_VECTOR_PROTECTION, general_protection);
    set_trap_gate(INT_VECTOR_PAGE_FAULT, page_fault);
    set_trap_gate(INT_VECTOR_COPROC_ERR, copr_error);
    // hard interrupt IRQ0-IRQ15
    set_intr_gate(INT_VECTOR_IRQ0 + 0,	 clock_intr); //clock interrupt
    set_intr_gate(INT_VECTOR_IRQ0 + 1,	 kb_intr); //keyboard interrupt
    set_intr_gate(INT_VECTOR_IRQ0 + 2,	 hwint02);
    set_intr_gate(INT_VECTOR_IRQ0 + 3,	 hwint03);
    set_intr_gate(INT_VECTOR_IRQ0 + 4,	 hwint04);
    set_intr_gate(INT_VECTOR_IRQ0 + 5,	 hwint05);
    set_intr_gate(INT_VECTOR_IRQ0 + 6,	 hwint06);
    set_intr_gate(INT_VECTOR_IRQ0 + 7,	 hwint07);
    set_intr_gate(INT_VECTOR_IRQ8 + 0,	 hwint08);
    set_intr_gate(INT_VECTOR_IRQ8 + 1,	 hwint09);
    set_intr_gate(INT_VECTOR_IRQ8 + 2,	 hwint10);
    set_intr_gate(INT_VECTOR_IRQ8 + 3,	 hwint11);
    set_intr_gate(INT_VECTOR_IRQ8 + 4,	 hwint12);
    set_intr_gate(INT_VECTOR_IRQ8 + 5,	 hwint13);
    set_intr_gate(INT_VECTOR_IRQ8 + 6,	 hd_intr);//hard disk interrupt
    set_intr_gate(INT_VECTOR_IRQ8 + 7,	 hwint15);
    //init 80 interrupt
    set_syscall_gate(INT_VECTOR_SYS_CALL,	 sys_call);

    memset((char *)&tss, 0, sizeof(tss));
    tss.ss0		= SELECTOR_KERNEL_DS;
    //¿¿TSS
    init_descriptor(&gdt[INDEX_TSS],
            vir2phys(seg2phys(SELECTOR_KERNEL_DS), &tss),
            sizeof(tss) - 1,
            DA_386TSS);
    tss.iobase	= sizeof(tss);	
    PROCESS* p_proc	= proc_table;
    t_16 selector_ldt = INDEX_LDT_FIRST << 3;

    //¿¿GDT¿¿¿¿¿¿¿LDT 
    for(i=0;i<NR_SYSTEM_PROCS + NR_USER_PROCS;i++)
    {	
        init_descriptor(&gdt[selector_ldt>>3],
                vir2phys(seg2phys(SELECTOR_KERNEL_DS), proc_table[i].ldts),
                LDT_SIZE * sizeof(DESCRIPTOR) - 1,
                DA_LDT);
        p_proc++;
        selector_ldt += 1 << 3;
    }
}


void init_idt_desc(unsigned int n, t_8 desc_type, t_pf_int_handler handler, unsigned char privilege)
{
    GATE *	p_gate	= &idt[n];
    t_32	base	= (t_32)handler;
    p_gate->offset_low	= base & 0xFFFF;
    p_gate->selector	= SELECTOR_KERNEL_CS;
    p_gate->dcount		= 0;
    p_gate->attr		= desc_type | (privilege << 5);
    p_gate->offset_high	= (base >> 16) & 0xFFFF;
}


static t_32 seg2phys(t_16 seg)
{
    DESCRIPTOR* p_dest = &gdt[seg >> 3];
    return (p_dest->base_high << 24) | (p_dest->base_mid << 16) | (p_dest->base_low);
}

void init_descriptor(DESCRIPTOR * p_desc, t_32 base, t_32 limit, t_16 attribute)
{
    p_desc->limit_low		= limit & 0x0FFFF;	
    p_desc->base_low		= base & 0x0FFFF;	
    p_desc->base_mid		= (base >> 16) & 0x0FF;	
    p_desc->attr1			= attribute & 0xFF;	
    p_desc->limit_high_attr2	= ((limit >> 16) & 0x0F) | ((attribute >> 8) & 0xF0);
    p_desc->base_high		= (base >> 24) & 0x0FF;	
}

void exception_handler(int vec_no, int err_code, int eip, int cs, int eflags)
{
    int i;
    int text_color = 0x74;
    char err_description[][64] = {	
        "#DE Divide Error",
        "#DB RESERVED",
        "¡ª  NMI Interrupt",
        "#BP Breakpoint",
        "#OF Overflow",
        "#BR BOUND Range Exceeded",
        "#UD Invalid Opcode (Undefined Opcode)",
        "#NM Device Not Available (No Math Coprocessor)",
        "#DF Double Fault",
        "    Coprocessor Segment Overrun (reserved)",
        "#TS Invalid TSS",
        "#NP Segment Not Present",
        "#SS Stack-Segment Fault",
        "#GP General Protection",
        "#PF Page Fault",
        "¡ª  (Intel reserved. Do not use.)",
        "#MF x87 FPU Floating-Point Error (Math Fault)",
        "#AC Alignment Check",
        "#MC Machine Check",
        "#XF SIMD Floating-Point Exception"
    };

    disp_pos = 0;

    for(i=0;i<80*5;i++){
        disp_str(" ");
    }
    disp_pos = 0;

    disp_color_str("Exception! --> ", text_color);
    disp_color_str(err_description[vec_no], text_color);
    disp_color_str("\n\n", text_color);
    disp_color_str("EFLAGS:", text_color);
    disp_int(eflags);
    disp_color_str("CS:", text_color);
    disp_int(cs);
    disp_color_str("EIP:", text_color);
    disp_int(eip);

    if(err_code != 0xFFFFFFFF)
    {
        disp_color_str("Error code:", text_color);
        disp_int(err_code);
    }
}

