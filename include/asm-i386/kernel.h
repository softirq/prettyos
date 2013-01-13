#ifndef     _KERNEL_H_
#define     _KERNEL_H_

#define     KERNEL_PRIOR    25
#define     USER_PRIO       5

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

/* core.asm */
extern void	restart();
extern void	move_to_user_mode();

extern	t8		gdt_ptr[6];	// 0~15:Limit  16~47:Base
extern	DESCRIPTOR	gdt[GDT_SIZE];
extern	t8		idt_ptr[6];	// 0~15:Limit  16~47:Base
extern	GATE		idt[IDT_SIZE];
extern unsigned short selector_ldt;

#define 	suser() (!current->euid == 0)

#endif
