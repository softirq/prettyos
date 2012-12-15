
%include "sconst.inc"

_NR_get_ticks		equ	0	
_NR_write		equ 	1
_NR_printx		equ 	2
_NR_fork 		equ 	3
_NR_exit 		equ 	4
_NR_waitpid 		equ 	5

INT_VECTOR_SYS_CALL	equ	0x80  ;syscall number

global	get_ticks
global 	write
global 	printx
global 	fork
global  exit
global  waitpid

bits 32
[section .text]

get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

write:
	mov 	eax,_NR_write
	mov 	ebx,[esp + 4]
	mov 	ecx,[esp + 8]
	int 	INT_VECTOR_SYS_CALL
	ret

printx:
	mov 	eax,_NR_printx
	mov 	ebx,[esp + 4]
	mov 	ecx,[esp + 8]
	int 	INT_VECTOR_SYS_CALL
	ret

fork:
	mov 	eax,_NR_fork
	int 	INT_VECTOR_SYS_CALL
	ret

exit:
	mov 	eax,_NR_exit
	int 	INT_VECTOR_SYS_CALL
	ret

waitpid:
	mov 	eax,_NR_waitpid
	mov 	ebx,[esp + 4]
	int 	INT_VECTOR_SYS_CALL
	ret

