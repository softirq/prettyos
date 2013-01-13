org  0100h
jmp	LABEL_START		

%include	"fat12hdr.inc"
%include	"load.inc"
%include	"pm.inc"

LABEL_GDT:			Descriptor             0,                    0, 0						
LABEL_DESC_FLAT_C:		Descriptor             0,              0fffffh, DA_CR  | DA_32 | DA_LIMIT_4K			
LABEL_DESC_FLAT_RW:		Descriptor             0,              0fffffh, DA_DRW | DA_32 | DA_LIMIT_4K			
;set DPL for 3 then user process could write and read
LABEL_DESC_VIDEO:		Descriptor	 0B8000h,               0ffffh, DA_DRW | DA_DPL3	

GdtLen		equ	$ - LABEL_GDT
GdtPtr		dw	GdtLen - 1				
		dd	BaseOfLoaderPhyAddr + LABEL_GDT		

SelectorFlatC		equ	LABEL_DESC_FLAT_C	- LABEL_GDT
SelectorFlatRW		equ	LABEL_DESC_FLAT_RW	- LABEL_GDT
SelectorVideo		equ	LABEL_DESC_VIDEO	- LABEL_GDT + SA_RPL3
BaseOfStack	equ	    0100h

LABEL_START:			
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, BaseOfStack
	mov	dh, 0			
	call	DispStrRealMode		
	
;获取内存信息
	mov	ebx, 0			
	mov	di, _MemChkBuf		

.MemChkLoop:
	mov	eax, 0E820h		
	mov	ecx, 20			
	mov	edx, 0534D4150h		
	int	15h			
	jc	.MemChkFail
	add	di, 20
	inc	dword [_dwMCRNumber]	
	cmp	ebx, 0
	jne	.MemChkLoop
	jmp	.MemChkOK

.MemChkFail:
	mov	dword [_dwMCRNumber], 0
.MemChkOK:
	
	mov	word [wSectorNo], SectorNoOfRootDirectory	
	xor	ah, ah	
	xor	dl, dl	
	int	13h	
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp	word [wRootDirSizeForLoop], 0	
	jz	LABEL_NO_KERNELBIN		
	dec	word [wRootDirSizeForLoop]	
	mov	ax, BaseOfKernelFile
	mov	es, ax			
	mov	bx, OffsetOfKernelFile	
	mov	ax, [wSectorNo]		
	mov	cl, 1
	call	ReadSector
	mov	si, KernelFileName	
	mov	di, OffsetOfKernelFile	
	cld
	mov	dx, 10h
LABEL_SEARCH_FOR_KERNELBIN:
	cmp	dx, 0					
	jz	LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR	
	dec	dx					
	mov	cx, 11
LABEL_CMP_FILENAME:
	cmp	cx, 0			
	jz	LABEL_FILENAME_FOUND	
	dec	cx			
	lodsb				
	cmp	al, byte [es:di]	
	jz	LABEL_GO_ON
	jmp	LABEL_DIFFERENT
LABEL_GO_ON:
	inc	di
	jmp	LABEL_CMP_FILENAME	
LABEL_DIFFERENT:
	and	di, 0FFE0h		
	add	di, 20h			
	mov	si, KernelFileName	
	jmp	LABEL_SEARCH_FOR_KERNELBIN
LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
	add	word [wSectorNo], 1
	jmp	LABEL_SEARCH_IN_ROOT_DIR_BEGIN
LABEL_NO_KERNELBIN:
	mov	dh, 2			
	call	DispStrRealMode		
	jmp	$			
LABEL_FILENAME_FOUND:			
	mov	ax, RootDirSectors
	and	di, 0FFF0h		
	push	eax
	mov	eax, [es : di + 01Ch]		
	mov	dword [dwKernelSize], eax	
	pop	eax
	add	di, 01Ah		
	mov	cx, word [es:di]
	push	cx			
	add	cx, ax
	add	cx, DeltaSectorNo	
	mov	ax, BaseOfKernelFile
	mov	es, ax			
	mov	bx, OffsetOfKernelFile	
	mov	ax, cx			
LABEL_GOON_LOADING_FILE:
	push	ax			
	push	bx			
	mov	ah, 0Eh			
	mov	al, '.'			
	mov	bl, 0Fh			
	int	10h			
	pop	bx			
	pop	ax			
	mov	cl, 1
	call	ReadSector
	pop	ax			
	call	GetFATEntry
	cmp	ax, 0FFFh
	jz	LABEL_FILE_LOADED
	push	ax			
	mov	dx, RootDirSectors
	add	ax, dx
	add	ax, DeltaSectorNo
	add	bx, [BPB_BytsPerSec]
	jmp	LABEL_GOON_LOADING_FILE
LABEL_FILE_LOADED:
	call	KillMotor		
	mov	dh, 1			
	call	DispStrRealMode		
	
	lgdt	[GdtPtr]
	cli
	in	al, 92h
	or	al, 00000010b
	out	92h, al

	mov	eax, cr0
	or	eax, 1
	mov	cr0, eax

	jmp	dword SelectorFlatC:(BaseOfLoaderPhyAddr+LABEL_PM_START)

wRootDirSizeForLoop	dw	RootDirSectors	
wSectorNo		dw	0		
bOdd			db	0		
dwKernelSize		dd	0		
KernelFileName		db	"KERNEL  BIN", 0	
MessageLength		equ	9
LoadMessage:		db	"Loading  "
Message1		db	"Ready.   "
Message2		db	"No KERNEL"

DispStrRealMode:
	mov	ax, MessageLength
	mul	dh
	add	ax, LoadMessage
	mov	bp, ax			
	mov	ax, ds			
	mov	es, ax			
	mov	cx, MessageLength	
	mov	ax, 01301h		
	mov	bx, 0007h		
	mov	dl, 0
	add	dh, 3			
	int	10h			
	ret
ReadSector:
	push	bp
	mov	bp, sp
	sub	esp, 2			
	mov	byte [bp-2], cl
	push	bx			
	mov	bl, [BPB_SecPerTrk]	
	div	bl			
	inc	ah			
	mov	cl, ah			
	mov	dh, al			
	shr	al, 1			
	mov	ch, al			
	and	dh, 1			
	pop	bx			
	mov	dl, [BS_DrvNum]		
.GoOnReading:
	mov	ah, 2			
	mov	al, byte [bp-2]		
	int	13h
	jc	.GoOnReading		
	add	esp, 2
	pop	bp
	ret
GetFATEntry:
	push	es
	push	bx
	push	ax
	mov	ax, BaseOfKernelFile	
	sub	ax, 0100h		
	mov	es, ax			
	pop	ax
	mov	byte [bOdd], 0
	mov	bx, 3
	mul	bx			
	mov	bx, 2
	div	bx			
	cmp	dx, 0
	jz	LABEL_EVEN
	mov	byte [bOdd], 1
LABEL_EVEN:
	xor	dx, dx			
	mov	bx, [BPB_BytsPerSec]
	div	bx			
					
	push	dx
	mov	bx, 0			
	add	ax, SectorNoOfFAT1	
	mov	cl, 2
	call	ReadSector		
	pop	dx
	add	bx, dx
	mov	ax, [es:bx]
	cmp	byte [bOdd], 1
	jnz	LABEL_EVEN_2
	shr	ax, 4
LABEL_EVEN_2:
	and	ax, 0FFFh
LABEL_GET_FAT_ENRY_OK:
	pop	bx
	pop	es
	ret
KillMotor:
	push	dx
	mov	dx, 03F2h
	mov	al, 0
	out	dx, al
	pop	dx
	ret
[SECTION .s32]
ALIGN	32
[BITS	32]
LABEL_PM_START:
	mov	ax, SelectorVideo
	mov	gs, ax
	mov	ax, SelectorFlatRW
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	ss, ax
	mov	esp, TopOfStack

;	call	DispReturn		
;	call	DispReturn		
;	call	DispReturn		
;	call	DispReturn		
;	push	szMemChkTitle
;	call	DispStr
;	add	esp, 4

	call	DispMemInfo

	call	SetupPaging
	call	InitKernel

	mov 	dword [BOOT_PARAM_ADDR],BOOT_PARAM_MAGIC

	mov 	eax,[dwMemSize]
	mov 	dword [BOOT_PARAM_ADDR + 4],eax

	mov 	eax,BaseOfKernelFilePhyAddr 
	mov 	dword [BOOT_PARAM_ADDR + 8],eax

	jmp	SelectorFlatC:KernelEntryPointPhyAddr	

DispAL:
	push	ecx
	push	edx
	push	edi
	mov	edi, [dwDispPos]
	mov	ah, 0Fh			
	mov	dl, al
	shr	al, 4
	mov	ecx, 2
.begin:
	and	al, 01111b
	cmp	al, 9
	ja	.1
	add	al, '0'
	jmp	.2
.1:
	sub	al, 0Ah
	add	al, 'A'
.2:
	mov	[gs:edi], ax
	add	edi, 2
	mov	al, dl
	loop	.begin
	
	mov	[dwDispPos], edi
	pop	edi
	pop	edx
	pop	ecx
	ret
DispInt:
	mov	eax, [esp + 4]
	shr	eax, 24
	call	DispAL
	mov	eax, [esp + 4]
	shr	eax, 16
	call	DispAL
	mov	eax, [esp + 4]
	shr	eax, 8
	call	DispAL
	mov	eax, [esp + 4]
	call	DispAL
	mov	ah, 07h			
	mov	al, 'h'
	push	edi
	mov	edi, [dwDispPos]
	mov	[gs:edi], ax
	add	edi, 4
	mov	[dwDispPos], edi
	pop	edi
	ret
DispStr:
	push	ebp
	mov	ebp, esp
	push	ebx
	push	esi
	push	edi

	mov	esi, [ebp + 8]	
	mov	edi, [dwDispPos]
	mov	ah, 0Fh
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1
.2:
	mov	[dwDispPos], edi
	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	ret
DispReturn:
	push	szReturn
	call	DispStr			
	add	esp, 4
	ret
MemCpy:
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

DispMemInfo:
	push	esi
	push	edi
	push	ecx
	push 	eax
	push 	edx

	mov	esi, MemChkBuf
	mov	ecx, [dwMCRNumber]	
.loop:					
	mov	edx, 5			
	mov	edi, ARDStruct		
.1:					
	push	dword [esi]		
;	call	DispInt			
	pop	eax			

	stosd				
	add	esi, 4			
	dec	edx			
	cmp	edx, 0			
	jnz	.1			

	call	DispReturn		

	cmp	dword [dwType], 1	
	jne	.2			
	mov	eax, [dwBaseAddrLow]	
	add	eax, [dwLengthLow]	
	cmp	eax, [dwMemSize]	
	jb	.2			
	mov	[dwMemSize], eax	
.2:					
	loop	.loop			
					
	call	DispReturn		
;	push	szRAMSize		
;	call	DispStr			
;	add	esp, 4			
					
;	push	dword [dwMemSize]	
;	call	DispInt			
;	add	esp, 4			

	pop 	edx
	pop 	eax
	pop	ecx
	pop	edi
	pop	esi
	ret

;only map the 0 1 and 0x300 0x301 entry exactly
SetupPaging:

;	push	dword [dwMemSize]	
;	call	DispInt			
;	add	esp, 4			
	
	xor	edx, edx
	mov	eax, [dwMemSize]
	mov	ebx, 400000h	
	div	ebx
	mov	ecx, eax	
	
;	push	ecx	
;	call	DispInt			
;	add	esp, 4			

	test	edx, edx
	jz	.no_remainder
	inc	ecx		

.no_remainder:
	push	ecx		

	mov	ax, SelectorFlatRW
	mov	es, ax
	mov	edi, PageDirBase	
	xor	eax, eax
	mov	eax, PageTblBase | PG_P  | PG_USU | PG_RWW

.1:
	stosd
	add	eax, 4096		
	loop	.1

	
	pop	eax			
	mov	ebx, 1024		
	mul	ebx
	mov	ecx, eax		

;	push	ecx	
;	call	DispInt			
;	add	esp, 4			

	mov	edi, PageTblBase	
	xor	eax, eax
	mov	eax, PG_P  | PG_USU | PG_RWW

.2:
	stosd
	add	eax, 4096		
	loop	.2

;	push	eax	
;	call	DispInt			
;	add	esp, 4			


	mov	eax, PageDirBase
	mov	cr3, eax

    ;open the pageing 
    ;set the cr0 the highest bit to 1

	mov	eax, cr0
	or	eax, 80000000h
	mov	cr0, eax
	jmp	short .3
.3:
	nop
	ret

InitKernel:	
	xor	esi, esi
	mov	cx, word [BaseOfKernelFilePhyAddr + 2Ch]
	movzx	ecx, cx					
	mov	esi, [BaseOfKernelFilePhyAddr + 1Ch]	
	add	esi, BaseOfKernelFilePhyAddr		
.Begin:
	mov	eax, [esi + 0]
	cmp	eax, 0				
	jz	.NoAction
	push	dword [esi + 010h]		
	mov	eax, [esi + 04h]		
	add	eax, BaseOfKernelFilePhyAddr	
	push	eax				
	push	dword [esi + 08h]		
	call	MemCpy				
	add	esp, 12				
.NoAction:
	add	esi, 020h			
	dec	ecx
	jnz	.Begin
	ret
[SECTION .data1]
ALIGN	32
LABEL_DATA:
_szMemChkTitle:			db	"BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
_szRAMSize:			db	"RAM size:", 0
_szReturn:			db	0Ah, 0
_dwMCRNumber:			dd	0	
_dwDispPos:			dd	(80 * 6 + 0) * 2	
_dwMemSize:			dd	0
_ARDStruct:			
	_dwBaseAddrLow:		dd	0
	_dwBaseAddrHigh:	dd	0
	_dwLengthLow:		dd	0
	_dwLengthHigh:		dd	0
	_dwType:		dd	0
_MemChkBuf:	times	256	db	0
szMemChkTitle		equ	BaseOfLoaderPhyAddr + _szMemChkTitle
szRAMSize		equ	BaseOfLoaderPhyAddr + _szRAMSize
szReturn		equ	BaseOfLoaderPhyAddr + _szReturn
dwDispPos		equ	BaseOfLoaderPhyAddr + _dwDispPos
dwMemSize		equ	BaseOfLoaderPhyAddr + _dwMemSize
dwMCRNumber		equ	BaseOfLoaderPhyAddr + _dwMCRNumber
ARDStruct		equ	BaseOfLoaderPhyAddr + _ARDStruct
	dwBaseAddrLow	equ	BaseOfLoaderPhyAddr + _dwBaseAddrLow
	dwBaseAddrHigh	equ	BaseOfLoaderPhyAddr + _dwBaseAddrHigh
	dwLengthLow	equ	BaseOfLoaderPhyAddr + _dwLengthLow
	dwLengthHigh	equ	BaseOfLoaderPhyAddr + _dwLengthHigh
	dwType		equ	BaseOfLoaderPhyAddr + _dwType
MemChkBuf		equ	BaseOfLoaderPhyAddr + _MemChkBuf
StackSpace:	times	1000h	db	0
TopOfStack	equ	BaseOfLoaderPhyAddr + $	
