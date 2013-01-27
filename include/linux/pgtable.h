#ifndef     _PGTABLE_H_
#define     _PGTABLE_H_

#include "page.h"
#include "asm-i386/pgtable.h"

#define     HIGH_MEM_ENTRY     0x300 
#define     PGD_ENTRYS       1024
#define     PTE_ENTRYS      (1024 * 1024)

#define     SAFE_GAP        4096 

//alloc low memory start and end address
extern unsigned long low_mem_start;
extern unsigned long low_mem_end;

extern unsigned long alloc_low_mem(int size);
#endif
