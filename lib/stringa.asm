[SECTION .text]
global	memcpy
global  mymemcpy
;global	memset
;global	strcpy
;global	strncpy
;global 	strlen
global 	strcmp
memcpy:
	push	ebp
	mov	ebp, esp
	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	
	mov	esi, [ebp + 12]	
	mov	ecx, [ebp + 16]	
.1:
	cmp	ecx, 0		
	jz	.2		
	mov	al, [ds:esi]		
	inc	esi			
					
	mov	byte [es:edi], al	
	inc	edi			
	dec	ecx		
	jmp	.1		
.2:
	mov	eax, [ebp + 8]	
	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp
	ret			
mymemcpy:
	push	ebp
	mov	ebp, esp
	push	esi
	push	edi
	push	ecx
	mov	edi, [ebp + 8]	
	mov	esi, [ebp + 12]	
	mov	ecx, [ebp + 16]	
.1:
	cmp	ecx, 0		
	jz	.2		
	mov	al, [ds:esi]		
	inc	esi			
					
	mov	byte [es:edi], al	
	inc	edi			
	dec	ecx		
	jmp	.1		
.2:
	mov	eax, [ebp + 8]	
	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp
;以字节为单位
memset:
	push	ebp
	mov	ebp, esp
	push	esi
	push	edi
	push	ecx
	mov	edi, [ebp + 8]	
	mov	edx, [ebp + 12]	
	mov	ecx, [ebp + 16]	
.1:
	cmp	ecx, 0		
	jz	.2		
	mov	byte [edi], dl		
	inc	edi			
	dec	ecx		
	jmp	.1		
.2:
	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp
	ret			

strcpy:
	push	ebp
	mov	ebp, esp
	mov	esi, [ebp + 12]	
	mov	edi, [ebp + 8]	
.1:
	mov	al, [esi]		
	inc	esi			
					
	mov	byte [edi], al		
	inc	edi			
	cmp	al, 0		
	jnz	.1		
	mov	eax, [ebp + 8]	
	pop	ebp
	ret			
;strncpy:
;	push	ebp
;	mov	ebp, esp
;	mov	esi, [ebp + 16]	
;	mov	edi, [ebp + 12]	
;	mov 	ecx, [ebp + 8]
;.1:
;	mov	al, [esi]		
;	inc	esi			
					
;	mov	byte [edi], al		
;	inc	edi			
;	dec 	ecx
;	cmp	al, 0		
;	jnz	.1		
;	cmp 	ecx,0
;	jnz	.1	
	
;	mov	eax, [ebp + 8]	
;	pop	ebp
;	ret			

strlen:
	push 	ebp
	mov 	ebp,esp

	mov 	eax,0
	mov 	esi,[ebp+8]
.1:
	cmp 	byte [esi],0
	jz	.2
	inc 	eax 
	inc 	esi
	jmp	.1

.2:
	pop 	ebp
	ret	;return eax

;strcmp:
;	push 	ebp
;	mov 	ebp,esp
;	mov 	esi,[ebp + 8]
;	mov 	edi,[ebp + 12]
;	mov 	ecx,[ebp + 16]
;	cld
;.2:		
;	cmpsb	
;	jz 	.1
;	mov	eax,-1	
;	ret 
	
;.1:
;	inc 	esi
;	inc 	edi		
;	cmp 	ecx,0	
;	jz 	.3
;	dec 	ecx
;	jmp 	.2
;.3:
;	mov 	eax,0
;	ret
	
