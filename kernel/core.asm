%include "sconst.inc"

extern	cstart
extern	pretty_main
extern	exception_handler
extern	spurious_irq
extern	clock_handler
extern	disp_str
extern 	disp_int
extern	delay
extern	gdt_ptr
extern	idt_ptr
extern  current	
extern	tss
extern	disp_pos
extern	k_reenter
extern	irq_table
extern	sys_call_table
extern 	get_signal_bitmap
extern  do_signal


bits 32
[SECTION .data]
clock_int_msg		db	"^", 0
[SECTION .bss]
StackSpace		resb	4 * 1024
StackTop:		
[section .text]	
global _start	
global	restart
global 	move_to_user_mode
global	sys_call

global	divide_error
global	single_step_exception
global	nmi
global	breakpoint_exception
global	overflow
global	bounds_check
global	inval_opcode
global	copr_not_available
global	double_fault
global	copr_seg_overrun
global	inval_tss
global	segment_not_present
global	stack_exception
global	general_protection
global	page_fault
global	copr_error
global 	clock_intr	
global 	kb_intr	
global	hwint02
global	hwint03
global	hwint04
global	hwint05
global	hwint06
global	hwint07
global	hwint08
global	hwint09
global	hwint10
global	hwint11
global	hwint12
global	hwint13
global 	hd_intr	
global	hwint15

_start:
;	jmp $
	cli
	mov	esp, StackTop	
	mov	dword [disp_pos], 0
	sgdt	[gdt_ptr]	
	call	cstart		
	lgdt	[gdt_ptr]	
	lidt	[idt_ptr]
	jmp	SELECTOR_KERNEL_CS:csinit
csinit:		
	xor	eax, eax
	mov	ax, SELECTOR_TSS
	ltr	ax
	jmp	pretty_main
	;进程调度之前开启中断
	sti

;_timer_interrupt:
;	push ds                ; save ds,es and put kernel data space
;	push es                ; into them. %fs is used by _system_call
;	push fs
;	push edx              ; we save %eax,%ecx,%edx as gcc doesn't
;	push ecx              ; save those across function calls. %ebx
;	push ebx              ; is saved as we use that in ret_sys_call
;	push eax
;	mov eax,0x10
;	mov ax,ds
;	mov ax,es
;	mov eax,0x17
;	mov ax,fs
;	incl _jiffies
;	mov al,0x20          ; EOI to interrupt controller #1
;	out 0x20,al
;	mov eax,CSreg(esp)
;	and eax,0x03            ; %eax is CPL (0 or 3, 0=supervisor)
;	push eax
;	call _do_timer          ; 'do_timer(long CPL)' does everything from
;	add esp,0x04            ; task switching to accounting ...
;	jmp ret_from_sys_call
;	
;hardware int

%macro	hwint_master	1
	call	save
	in	al, INT_M_CTLMASK	
	or	al, (1 << %1)		
	out	INT_M_CTLMASK, al	
	mov	al, EOI			
	out	INT_M_CTL, al		
;CPU在执行中断之前，默认是关中断的。可以手工开起中断，允许嵌套
	sti	
	push	%1			
	call	[irq_table + 4 * %1]	
	pop	ecx			
	cli
	in	al, INT_M_CTLMASK	
	and	al, ~(1 << %1)		
	out	INT_M_CTLMASK, al	
	ret
%endmacro

%macro hwint_slave	1
	call 	save
	in 	al,INT_S_CTLMASK
	or 	al,(1 << (%1 - 8))
	out 	INT_S_CTLMASK,al
	mov 	al,EOI
	out	INT_S_CTL,al
	sti
	push 	%1
	call	[irq_table + 4 * %1]
	pop 	ecx
	cli
	in 	al,INT_S_CTLMASK
	and 	al,~(1 << (%1 - 8))
	out	INT_S_CTLMASK,al
	ret
%endmacro

ALIGN	16
clock_intr:		
	hwint_master	0
ALIGN	16
kb_intr:		
	hwint_master	1
ALIGN	16
hwint02:		
	hwint_master	2
ALIGN	16
hwint03:		
	hwint_master	3
ALIGN	16
hwint04:		
	hwint_master	4
ALIGN	16
hwint05:		
	hwint_master	5
ALIGN	16
hwint06:		
	hwint_master	6
ALIGN	16
hwint07:		
	hwint_master	7
ALIGN	16
hwint08:		
	hwint_slave	8
ALIGN	16
hwint09:		
	hwint_slave	9
ALIGN	16
hwint10:		
	hwint_slave	10
ALIGN	16
hwint11:		
	hwint_slave	11
ALIGN	16
hwint12:		
	hwint_slave	12
ALIGN	16
hwint13:		
	hwint_slave	13
ALIGN	16
hd_intr:		
	hwint_slave	14
ALIGN	16
hwint15:		
	hwint_slave	15
divide_error:
	push	0xFFFFFFFF	
	push	0		
	jmp	exception
single_step_exception:
	push	0xFFFFFFFF	
	push	1		
	jmp	exception
nmi:
	push	0xFFFFFFFF	
	push	2		
	jmp	exception
breakpoint_exception:
	push	0xFFFFFFFF	
	push	3		
	jmp	exception
overflow:
	push	0xFFFFFFFF	
	push	4		
	jmp	exception
bounds_check:
	push	0xFFFFFFFF	
	push	5		
	jmp	exception
inval_opcode:
	push	0xFFFFFFFF	
	push	6		
	jmp	exception
copr_not_available:
	push	0xFFFFFFFF	
	push	7		
	jmp	exception
double_fault:
	push	8		
	jmp	exception
copr_seg_overrun:
	push	0xFFFFFFFF	
	push	9		
	jmp	exception
inval_tss:
	push	10		
	jmp	exception
segment_not_present:
	push	11		
	jmp	exception
stack_exception:
	push	12		
	jmp	exception
general_protection:
	push	13		
	jmp	exception
page_fault:
	push	14		
	jmp	exception
copr_error:
	push	0xFFFFFFFF	
	push	16		
	jmp	exception
exception:
	call	exception_handler
	add	esp, 4*2	
	hlt

save:
	;关中断中进行
	pushad		
	push	ds	
	push	es	
	push	fs	
	push	gs	

	mov	dx, ss
	mov	ds, dx
	mov	es, dx
	mov	esi, esp			

	inc	dword [k_reenter]		
	cmp	dword [k_reenter], 0		
	jne	.1				

	mov	esp, StackTop			
	push	restart				
	jmp	[esi + RETADR - P_STACKBASE]	
.1:						
	push	restart_reenter			
	jmp	[esi + RETADR - P_STACKBASE]	
						
restart:
	mov	esp, [current];进程切换
	lldt	[esp + P_LDT_SEL] 
	;保存sp0堆栈
	lea	eax, [esp + P_STACKTOP]
	mov	dword [tss + TSS3_S_SP0], eax
restart_reenter:
	dec	dword [k_reenter]
ret_from_sys_call:

;	xor 	ecx,ecx
;	call	get_signal_bitmap
;	bsf	ecx,eax
;	cmp 	ecx,0
;	jz 	_no_signal	
;	push  	ecx	
;	call 	do_signal
;	add 	esp,4
;_no_signal:

	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad

	add	esp, 4
	iretd

sys_call:
	call	save
	sti

	push 	dword [current]
	push 	ecx
	push 	ebx
	call	[sys_call_table + eax * 4]
	add 	esp,4 * 3

	;return value
	mov	[esi + EAXREG - P_STACKBASE], eax
	cli
	ret

move_to_user_mode:
        mov     esp, [current];进程切换
        lldt    [esp + P_LDT_SEL]
        ;保存sp0堆栈
        lea     eax, [esp + P_STACKTOP]
        mov     dword [tss + TSS3_S_SP0], eax
       	dec     dword [k_reenter]
        pop     gs
        pop     fs
        pop     es
        pop     ds
        popad   
        
        add     esp, 4
        iretd   
