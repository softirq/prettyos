#ifndef     _I386_PAGE_H_
#define     _I386_PAGE_H_

#include "list.h"
#include "wait.h"
#include "atomic.h"

typedef unsigned int mem_map_t;

typedef struct { unsigned long pte; } pte_t;
typedef struct { unsigned long pmd; } pmd_t;
typedef struct { unsigned long pgd; } pgd_t;
typedef struct { unsigned long pgprot; } pgprot_t;


#define pgd_val(x)  ((x).pgd)
#define pmd_val(x)  ((x).pmd)
#define pte_val(x)  ((x).pte)

#define ppgd_val(x)  ((x)->pgd)
#define ppmd_val(x)  ((x)->pmd)
#define ppte_val(x)  ((x)->pte)

#define pgprot_val(x)   ((x).pgprot)

#define PAGE_SHIFT 			12

#define PTE_SHIFT           12
#define PMD_SHIFT           22
#define PGD_SHIFT 		22

#define PGD_SIZE 			(1UL << PGD_SHIFT)
#define PMD_SIZE 			PGD_SIZE
#define PGD_MASK 			(~(PGD_SIZE - 1))
#define PMD_MASK 			PGD_MASK

#define __PAGE_OFFSET 		(0xC0000000UL)

#define PAGE_SIZE 			(1UL << PAGE_SHIFT)
#define PAGE_MASK 			(~(PAGE_SIZE - 1))

#define __pte(x)        ((pte_t) { (x) } )
#define __pmd(x)        ((pmd_t) { (x) } )
#define __pgd(x)        ((pgd_t) { (x) } )
#define __pgprot(x)     ((pgprot_t) { (x) } )

#define 	virt_to_page(addr)  	 (mem_map + ((unsigned long)(addr - PAGE_OFFSET) >> PAGE_SHIFT))
#define 	page_to_virt(page) 		 (((page - mem_map) >> PAGE_SHIFT) + PAGE_OFFSET)

#define 	PAGE_ALIGN(addr) 		( (addr + PAGE_SIZE -1) & PAGE_MASK )

#define MAP_PAGE_RESERVED 	(1 << 15 )

#define PAGE_OFFSET 	(unsigned long)__PAGE_OFFSET
#define __pa(x) 	((unsigned long)(x) - PAGE_OFFSET)
#define __va(x) 	((unsigned long)(x) + PAGE_OFFSET)

struct address_space;

typedef struct page
{
    struct list_head list;
    struct address_space *mapping;
    unsigned long address;
    unsigned long index;
    struct page *next_hash;
    //int count;
    atomic_t count;
    unsigned long flags;
    struct list_head lru;
    wait_queue_head_t wait;
    struct buffer_head *buffers;
    struct zone_struct *zone;
}Page;

#endif
